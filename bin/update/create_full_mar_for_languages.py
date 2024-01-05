#!/usr/bin/env python3

import sys
import os
import subprocess
import json

from tools import uncompress_file_to_dir, get_file_info

from path import UpdaterPath
from signing import sign_mar_file


def make_complete_mar_name(target_dir, filename_prefix, language):
    filename = filename_prefix + "_" + language + "_complete_langpack.mar"
    return os.path.join(target_dir, filename)


def create_lang_infos(mar_file_name, language, url):
    data = {'lang': language,
            'complete': get_file_info(mar_file_name, url)
            }
    return data


def main():
    if len(sys.argv) < 8:
        print(
            "Usage: create_full_mar_for_languages.py $PRODUCTNAME $WORKDIR $TARGETDIR $TEMPDIR $FILENAMEPREFIX $CERTIFICATEPATH $CERTIFICATENAME $BASEURL $VERSION")
        sys.exit(1)

    certificate_path = sys.argv[4]
    certificate_name = sys.argv[5]
    base_url = sys.argv[6]
    filename_prefix = sys.argv[3]
    workdir = sys.argv[2]
    product_name = sys.argv[1]
    version = sys.argv[7]

    updater_path = UpdaterPath(workdir)
    target_dir = updater_path.get_update_dir()
    temp_dir = updater_path.get_language_dir()

    language_pack_dir = os.path.join(workdir, "installation", product_name + "_languagepack", "archive", "install")
    language_packs = os.listdir(language_pack_dir)
    lang_infos = []
    for language in language_packs:
        if language == 'log':
            continue

        language_dir = os.path.join(language_pack_dir, language)
        language_file = os.path.join(language_dir, os.listdir(language_dir)[0])

        directory = uncompress_file_to_dir(language_file, os.path.join(temp_dir, language))

        mar_file_name = make_complete_mar_name(target_dir, filename_prefix, language)

        os.putenv('MOZ_PRODUCT_VERSION', version)
        os.putenv('MAR_CHANNEL_ID', 'LOOnlineUpdater')
        subprocess.call([os.path.join(workdir, 'UnpackedTarball/onlineupdate/tools/update-packaging/make_full_update.sh'), mar_file_name, directory])

        sign_mar_file(target_dir, certificate_path, certificate_name, mar_file_name, filename_prefix)

        lang_infos.append(create_lang_infos(mar_file_name, language, base_url))

    with open(os.path.join(target_dir, "complete_lang_info.json"), "w") as language_info_file:
        json.dump({'languages': lang_infos}, language_info_file, indent=4)


if __name__ == '__main__':
    main()
