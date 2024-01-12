#!/usr/bin/env python3
import glob
import json
import os
import re
import subprocess
import sys

from path import UpdaterPath, convert_to_native
from signing import sign_mar_file
from tools import get_file_info, uncompress_file_to_dir


def generate_file_name(old_build_id, mar_name_prefix):
    name = "%s_from_%s_partial.mar" % (mar_name_prefix, old_build_id)
    return name


def main():
    workdir = sys.argv[1]

    updater_path = UpdaterPath(workdir)
    updater_path.ensure_dir_exist()

    mar_name_prefix = sys.argv[2]
    channel = sys.argv[3]
    certificate_path = sys.argv[4]
    certificate_name = sys.argv[5]
    base_url = sys.argv[6]
    product_name = sys.argv[7]
    version = sys.argv[8]
    old_archive = sys.argv[9]
    old_metadata = sys.argv[10]

    old_uncompress_dir = uncompress_file_to_dir(old_archive, updater_path.get_previous_build_dir())
    versionini = os.path.join(old_uncompress_dir, 'program', 'version.ini') #TODO: Linux, macOS
    old_build_id = None
    with open(versionini) as f:
        for l in f:
            m = re.fullmatch('buildid=(.*)', l.rstrip())
            if m:
                old_build_id = m.group(1)
                break
    if old_build_id is None:
        raise Exception(f'Cannot find buildid in {versionini}')

    #TODO: check for problematic changes between old and new metadata
    with open(old_metadata) as meta:
        for l in meta:
            m = re.fullmatch('skip (.*)', l.rstrip())
            if m and m.group(1).startswith(f'{product_name}/'):
                path = m.group(1)[len(f'{product_name}/'):]
                os.remove(os.path.join(old_uncompress_dir, path))

    new_tar_file_glob = os.path.join(updater_path.get_workdir(), "installation", product_name, "archive", "install", "*", f'{product_name}_*_archive*')
    new_tar_files = glob.glob(new_tar_file_glob)
    if len(new_tar_files) != 1:
        raise Exception(f'`{new_tar_file_glob}` does not match exactly one file')
    new_tar_file = new_tar_files[0]
    new_uncompress_dir = uncompress_file_to_dir(new_tar_file, updater_path.get_current_build_dir())

    new_metadata = os.path.join(
        updater_path.get_workdir(), 'installation', product_name, 'archive', 'install', 'metadata')
    ifsfile = os.path.join(updater_path.get_mar_dir(), 'ifs')
    with open(new_metadata) as meta, open(ifsfile, 'w') as ifs:
        for l in meta:
            m = re.fullmatch('(skip|cond) (.*)', l.rstrip())
            if m and m.group(2).startswith(f'{product_name}/'):
                path = m.group(2)[len(f'{product_name}/'):]
                if m.group(1) == 'skip':
                    os.remove(os.path.join(new_uncompress_dir, path))
                else:
                    ifs.write(f'"{path}" "{path}"\n')

    update_dir = updater_path.get_update_dir()

    file_name = generate_file_name(old_build_id, mar_name_prefix)
    mar_file = os.path.join(update_dir, file_name)

    os.putenv('MOZ_PRODUCT_VERSION', version)
    os.putenv('MAR_CHANNEL_ID', 'LOOnlineUpdater')
    subprocess.call([os.path.join(workdir, 'UnpackedTarball/onlineupdate/tools/update-packaging/make_incremental_update.sh'), convert_to_native(mar_file),
                     convert_to_native(old_uncompress_dir), convert_to_native(new_uncompress_dir),
                     convert_to_native(ifsfile)])

    sign_mar_file(update_dir, certificate_path, certificate_name, mar_file, mar_name_prefix)

    data = {
        'from': old_build_id,
        'see also': '',
        'update': get_file_info(mar_file, base_url),
        'languages': {}
    };
    with open(os.path.join(update_dir, channel), "w") as f:
        json.dump(data, f, indent=4)


if __name__ == '__main__':
    main()
