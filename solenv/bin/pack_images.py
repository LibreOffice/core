# -*- coding: utf-8 -*-
# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
""" Pack images into archives. """

from __future__ import with_statement

import argparse
from collections import OrderedDict
import contextlib
import logging
import os
import shutil
import sys
import tempfile
import zipfile


logging.basicConfig(format='%(message)s')
LOGGER = logging.getLogger()


def main(args):
    """ Main function. """

    tmp_output_file = "%s.%d.tmp" % (args.output_file, os.getpid())

    # Sanity checks
    if not os.path.exists(args.imagelist_file):
        LOGGER.error("imagelist_file '%s' doesn't exists", args.imagelist_file)
        sys.exit(2)

    if args.links_file is not None and not os.path.exists(args.links_file):
        LOGGER.error("link_file '%s' doesn't exists", args.links_file)
        sys.exit(2)

    out_path = os.path.dirname(args.output_file)
    for path in (out_path, args.global_path, args.module_path):
        if not os.path.exists(path):
            LOGGER.error("Path '%s' doesn't exists", path)
            sys.exit(2)
        if not os.access(path, os.X_OK):
            LOGGER.error("Unable to search path %s", path)
            sys.exit(2)

    if not os.access(out_path, os.W_OK):
        LOGGER.error("Unable to write into path %s", out_path)

    custom_paths = []
    for path in args.custom_path:
        if not os.path.exists(path):
            LOGGER.warning("Skipping non-existing path: %s", path)
            continue
        elif not os.access(path, os.X_OK):
            LOGGER.error("Unable to search path %s", path)
            sys.exit(2)

        custom_paths.append(path)

    imagelist_filenames = get_imagelist_filenames(args.imagelist_file)
    global_image_list, module_image_list = parse_image_list(imagelist_filenames)
    custom_image_list = find_custom(custom_paths)

    links = {}
    if args.links_file is not None:
        read_links(links, args.links_file)
    else:
        read_links(links, os.path.join(ARGS.global_path, "links.txt"))
        for path in custom_paths:
            read_links(links, os.path.join(path, "links.txt"))
    check_links(links)

    zip_list = create_zip_list(global_image_list, module_image_list, custom_image_list,
                               args.global_path, args.module_path)
    remove_links_from_zip_list(zip_list, links)

    if check_rebuild(args.output_file, imagelist_filenames, custom_paths, zip_list, args.links_file):
        tmp_dir = copy_images(zip_list)
        create_zip_archive(zip_list, links, tmp_dir, tmp_output_file, args.sort_file)

        replace_zip_file(tmp_output_file, args.output_file)
        try:
            LOGGER.info("Remove temporary directory %s", tmp_dir)
            shutil.rmtree(tmp_dir)
        except Exception as e:
            LOGGER.error("Unable to remove temporary directory %s", tmp_dir)
            sys.exit(2)
    else:
        LOGGER.info("No rebuild needed. %s is up to date.", args.output_file)


def check_rebuild(zip_file, imagelist_filenames, custom_paths, zip_list, links_file):
    """ Check if a rebuild is needed.

    :type zip_file: str
    :param zip_file: File to check against (use st_mtime).

    :type imagelist_filenames: dict
    :param imagelist_filenames: List of imagelist filename.

    :type custom_paths: list
    :param custom_paths: List of paths to use with links.txt files.

    :type zip_list: dict
    :param zip_list: List of filenames to create the zip archive.

    :type links_file: str
    :param links_file: filename to read the links from

    :rtype: bool
    :return: True if rebuild is needed and False if not.
    """

    if not os.path.exists(zip_file):
        return True

    zip_file_stat = os.stat(zip_file)

    def compare_modtime(filenames):
        """ Check if modification time of zip archive is older the provided
        list of files.

        :type filenames: dict
        :param filenames: List of filenames to check against.

        :rtype: bool
        :return: True if zip archive is older and False if not.
        """
        for filename, path in filenames.items():
            filename = os.path.join(path, filename) if filename else path
            if os.path.exists(filename) and zip_file_stat.st_mtime < os.stat(filename).st_mtime:
                return True
        return False

    if compare_modtime(imagelist_filenames):
        return True
    if compare_modtime(zip_list):
        return True
    if links_file is not None:
        if zip_file_stat.st_mtime < os.stat(links_file).st_mtime:
            return True
    else:
        for path in custom_paths:
            link_file = os.path.join(path, 'links.txt')
            if os.path.exists(link_file):
                if zip_file_stat.st_mtime < os.stat(link_file).st_mtime:
                    return True

    return False


def replace_zip_file(src, dst):
    """ Replace the old archive file with the newly created one.

    :type src: str
    :param src: Source file name.

    :type dst: str
    :param dst: Destination file name.
    """
    LOGGER.info("Replace old zip archive with new archive")
    if os.path.exists(dst):
        try:
            os.unlink(dst)
        except OSError as e:
            if os.path.exists(src):
                os.unlink(src)

            LOGGER.error("Unable to unlink old archive '%s'", dst)
            LOGGER.error(str(e))
            sys.exit(1)

    try:
        LOGGER.info("Copy archive '%s' to '%s'", src, dst)
        shutil.move(src, dst)
    except (shutil.SameFileError, OSError) as e:
        os.unlink(src)
        LOGGER.error("Cannot copy file '%s' to %s: %s", src, dst, str(e))
        sys.exit(1)


def optimize_zip_layout(zip_list, sort_file=None):
    """ Optimize the zip layout by ordering the list of filename alphabetically
    or with provided sort file (can be partly).

    :type zip_list: dict
    :param zip_list: List of files to include in the zip archive.

    :type sort_file: str
    :param sort_file: Path/Filename to file with sort order.

    :rtype: OrderedDict
    :return: Dict with right sort order.
    """
    if sort_file is None:
        LOGGER.info("No sort file provided")
        LOGGER.info("Sorting entries alphabetically")

        return OrderedDict(sorted(zip_list.items(), key=lambda t: t[0]))

    LOGGER.info("Sort entries from file '%s'", sort_file)
    sorted_zip_list = OrderedDict()
    try:
        fh = open(sort_file)
    except IOError:
        LOGGER.error("Cannot open file %s", sort_file)
        sys.exit(1)
    else:
        with fh:
            for line in fh:
                line = line.strip()
                if line == '' or line.startswith('#'):
                    continue

                if line in zip_list:
                    sorted_zip_list[line] = ''
                else:
                    LOGGER.warning("Unknown file '%s'", line)

    for key in sorted(zip_list.keys()):
        if key not in sorted_zip_list:
            sorted_zip_list[key] = ''

    return sorted_zip_list


def create_zip_archive(zip_list, links, tmp_dir, tmp_zip_file, sort_file=None):
    """ Create the zip archive.

    :type zip_list: dict
    :param zip_list: All filenames to be included in the archive.

    :type links: dict
    :param links: All filenames to create links.txt file.

    :type tmp_dir: str
    :param tmp_dir: Path to temporary saved files.

    :type tmp_zip_file: str
    :param tmp_zip_file: Filename/Path of temporary zip archive.

    :type sort_file: str|None
    :param sort_file: Optional filename with sort order to apply.
    """
    LOGGER.info("Creating image archive")

    old_pwd = os.getcwd()
    os.chdir(tmp_dir)

    ordered_zip_list = optimize_zip_layout(zip_list, sort_file)

    with contextlib.closing(zipfile.ZipFile(tmp_zip_file, 'w')) as tmp_zip:
        if links.keys():
            LOGGER.info("Add file 'links.txt' to zip archive")
            create_links_file(tmp_dir, links)
            tmp_zip.write('links.txt', compress_type=zipfile.ZIP_DEFLATED)

        for link in ordered_zip_list:
            LOGGER.info("Add file '%s' from path '%s' to zip archive", link, tmp_dir)
            try:
                if (link.endswith(".svg")):
                    tmp_zip.write(link, compress_type=zipfile.ZIP_DEFLATED)
                else:
                    tmp_zip.write(link)
            except OSError:
                LOGGER.warning("Unable to add file '%s' to zip archive", link)

    os.chdir(old_pwd)


def create_links_file(path, links):
    """ Create file links.txt. Contains all links.

    :type path: str
    :param path: Path where to create the file.

    :type links: dict
    :param links: Dict with links (source -> target).
    """
    LOGGER.info("Create file links.txt")

    try:
        filename = os.path.join(path, 'links.txt')
        fh = open(filename, 'w')
    except IOError:
        LOGGER.error("Cannot open file %s", filename)
        sys.exit(1)
    else:
        with fh:
            for key in sorted(links.keys()):
                fh.write("%s %s\n" % (key, links[key]))


def copy_images(zip_list):
    """ Create a temporary directory and copy images to that directory.

    :type zip_list: dict
    :param zip_list: Dict with all files.

    :rtype: str
    :return: Path of the temporary directory.
    """
    LOGGER.info("Copy image files to temporary directory")

    tmp_dir = tempfile.mkdtemp()
    for key, value in zip_list.items():
        path = os.path.join(value, key)
        out_path = os.path.join(tmp_dir, key)

        LOGGER.debug("Copying '%s' to '%s'", path, out_path)
        if os.path.exists(path):
            dirname = os.path.dirname(out_path)
            if not os.path.exists(dirname):
                os.makedirs(dirname)

            try:
                shutil.copyfile(path, out_path)
            except (shutil.SameFileError, OSError) as e:
                LOGGER.error("Cannot add file '%s' to image dir: %s", path, str(e))
                sys.exit(1)

    LOGGER.debug("Temporary directory '%s'", tmp_dir)
    return tmp_dir


def remove_links_from_zip_list(zip_list, links):
    """ Remove any files from zip list that are linked.

    :type zip_list: dict
    :param zip_list: Files to include in the zip archive.

    :type links: dict
    :param links: Images which are linked.
    """
    LOGGER.info("Remove linked files from zip list")

    for link in links.keys():
        if link in zip_list:
            del zip_list[link]

    LOGGER.debug("Cleaned zip list:\n%s", "\n".join(zip_list))


def create_zip_list(global_image_list, module_image_list, custom_image_list, global_path, module_path):
    """ Create list of images for the zip archive.

    :type global_image_list: dict
    :param global_image_list: Global images.

    :type module_image_list: dict
    :param module_image_list: Module images.

    :type custom_image_list: dict
    :param custom_image_list: Custom images.

    :type global_path: str
    :param global_path: Global path (use when no custom path is available).

    :type module_path: str
    :param module_path: Module path (use when no custom path is available).

    :rtype: dict
    :return: List of images to include in zip archive.
    """
    LOGGER.info("Assemble image list")

    zip_list = {}
    duplicates = []

    for key in global_image_list.keys():
        if key in module_image_list:
            duplicates.append(key)
            continue

        if key in custom_image_list:
            zip_list[key] = custom_image_list[key]
            continue

        zip_list[key] = global_path

    for key in module_image_list.keys():
        if key in custom_image_list:
            zip_list[key] = custom_image_list[key]
            continue

        zip_list[key] = module_path

    if duplicates:
        LOGGER.warning("Found duplicate entries in 'global' and 'module' list")
        LOGGER.warning("\n".join(duplicates))

    LOGGER.debug("Assembled image list:\n%s", "\n".join(zip_list))
    return zip_list


def check_links(links):
    """ Check that no link points to another link.

    :type links: dict
    :param links: Links to icons
    """

    stop = False

    for link, target in links.items():
        if target in links.keys():
            LOGGER.error("Link %s -> %s -> %s", link, target, links[target])
            stop = True

    if stop:
        LOGGER.error("Some icons in links.txt were found to link to other linked icons")
        sys.exit(1)


def read_links(links, filename):
    """ Read links from file.

    :type links: dict
    :param links: Hash to store all links

    :type filename: str
    :param filename: filename to read the links from
    """

    LOGGER.info("Read links from file '%s'", filename)
    if not os.path.isfile(filename):
        LOGGER.info("No file to read")
        return

    try:
        fh = open(filename)
    except IOError:
        LOGGER.error("Cannot open file %s", filename)
        sys.exit(1)
    else:
        with fh:
            for line in fh:
                line = line.strip()
                if line == '' or line.startswith('#'):
                    continue

                tmp = line.split(' ')
                if len(tmp) == 2:
                    links[tmp[0]] = tmp[1]
                else:
                    LOGGER.error("Malformed links line '%s' in file %s", line, filename)
                    sys.exit(1)

    LOGGER.debug("Read links:\n%s", "\n".join(links))


def find_custom(custom_paths):
    """ Find all custom images.

    :type custom_paths: list
    :param custom_paths: List of custom paths to search within.

    :rtype: dict
    :return: List of all custom images.
    """
    LOGGER.info("Find all images in custom paths")

    custom_image_list = {}
    for path in custom_paths:
        # find all png files under the path including subdirs
        custom_files = [val for sublist in [
            [os.path.join(i[0], j) for j in i[2]
             if j.endswith('.png') and os.path.isfile(os.path.join(i[0], j))]
            for i in os.walk(path)] for val in sublist]

        for filename in custom_files:
            if filename.startswith(path):
                key = filename.replace(os.path.join(path, ''), '')
                if key not in custom_image_list:
                    custom_image_list[key] = path

    LOGGER.debug("Custom image list:\n%s", "\n".join(custom_image_list.keys()))
    return custom_image_list


def parse_image_list(imagelist_filenames):
    """ Parse and extract filename from the imagelist files.

    :type imagelist_filenames: list
    :param imagelist_filenames: List of imagelist files.

    :rtype: tuple
    :return: Tuple with dicts containing the global or module image list.
    """

    global_image_list = {}
    module_image_list = {}
    for imagelist_filename in imagelist_filenames.keys():
        LOGGER.info("Parsing '%s'", imagelist_filename)

        try:
            fh = open(imagelist_filename)
        except IOError as e:
            LOGGER.error("Cannot open imagelist file %s", imagelist_filename)
            sys.exit(2)
        else:
            line_count = 0
            with fh:
                for line in fh:
                    line_count += 1
                    line = line.strip()
                    if line == '' or line.startswith('#'):
                        continue
                    # clean up backslashes and double slashes
                    line = line.replace('\\', '/')
                    line = line.replace('//', '/')

                    if line.startswith('%GLOBALRES%'):
                        key = "res/%s" % line.replace('%GLOBALRES%/', '')
                        global_image_list[key] = True
                        if key.endswith('.png'):
                            global_image_list[key[:-4] + '.svg'] = True
                        continue

                    if line.startswith('%MODULE%'):
                        key = line.replace('%MODULE%/', '')
                        module_image_list[key] = True
                        if key.endswith('.png'):
                            module_image_list[key[:-4] + '.svg'] = True
                        continue

                    LOGGER.error("Cannot parse line %s:%d", imagelist_filename, line_count)
                    sys.exit(1)

    LOGGER.debug("Global image list:\n%s", "\n".join(global_image_list))
    LOGGER.debug("Module image list:\n%s", "\n".join(module_image_list))
    return global_image_list, module_image_list


def get_imagelist_filenames(filename):
    """ Extract a list of imagelist filenames.

    :type filename: str
    :param filename: Name of file from extracting the list.

    :rtype: dict
    :return: List of imagelist filenames.
    """
    LOGGER.info("Extract the imagelist filenames")

    imagelist_filenames = {}
    try:
        fh = open(filename)
    except IOError:
        LOGGER.error("Cannot open imagelist file %s", filename)
        sys.exit(1)
    else:
        with fh:
            for line in fh:
                line = line.strip()
                if not line or line == '':
                    continue

                for line_split in line.split(' '):
                    line_split.strip()
                    imagelist_filenames[line_split] = ''

    LOGGER.debug("Extraced imagelist filenames:\n%s", "\n".join(imagelist_filenames.keys()))
    return imagelist_filenames


if __name__ == '__main__':
    parser = argparse.ArgumentParser("Pack images into archives")
    parser.add_argument('-o', '--output-file', dest='output_file',
                        action='store', required=True,
                        help='path to output archive')
    parser.add_argument('-l', '--imagelist-file', dest='imagelist_file',
                        action='store', required=True,
                        help='file containing list of image list file')
    parser.add_argument('-L', '--links-file', dest='links_file',
                        action='store', required=False,
                        help='file containing linked images')
    parser.add_argument('-s', '--sort-file', dest='sort_file',
                        action='store', required=True, default=None,
                        help='image sort order file')
    parser.add_argument('-c', '--custom-path', dest='custom_path',
                        action='append', required=True,
                        help='path to custom path directory')
    parser.add_argument('-g', '--global-path', dest='global_path',
                        action='store', required=True,
                        help='path to global images directory')
    parser.add_argument('-m', '--module-path', dest='module_path',
                        action='store', required=True,
                        help='path to module images directory')
    parser.add_argument('-v', '--verbose', dest='verbose',
                        action='count', default=0,
                        help='set the verbosity (can be used multiple times)')

    ARGS = parser.parse_args()
    LOGGER.setLevel(logging.ERROR - (10 * ARGS.verbose if ARGS.verbose <= 3 else 3))

    main(ARGS)
    sys.exit(0)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
