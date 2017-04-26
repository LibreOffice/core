#!/usr/bin/env python3
import requests
import json
import sys
import hashlib
import os
import subprocess
import errno
import json

from config import parse_config
from uncompress_mar import extract_mar
from tools import get_file_info
from signing import sign_mar_file

BUF_SIZE = 1024
current_dir_path = os.path.dirname(os.path.realpath(__file__))

def download_file(filepath, url, hash_string):
    with open(filepath, "wb") as f:
        response = requests.get(url, stream=True)

        if not response.ok:
            return

        for block in response.iter_content(1024):
            f.write(block)

    with open(filepath, "rb") as f:
        sha512 = hashlib.sha512()
        while True:
            data = f.read(BUF_SIZE)
            if not data:
                break
            sha512.update(data)
        file_hash = sha512.hexdigest()

    if file_hash != hash_string:
        pass

def mkdir_p(path):
    try:
        os.makedirs(path)
    except OSError as exc:  # Python >2.5
        if exc.errno == errno.EEXIST and os.path.isdir(path):
            pass
        else:
            raise

def handle_language(lang_entries, filedir):
    mar = os.environ.get('MAR', 'mar')
    langs = {}
    for lang, data in lang_entries.items():
        lang_dir = os.path.join(filedir, lang)
        lang_file = os.path.join(lang_dir, "lang.mar")
        mkdir_p(lang_dir)
        download_file(lang_file , data["url"], data["hash"])
        dir_path = os.path.join(lang_dir, "lang")
        mkdir_p(dir_path)
        extract_mar(lang_file, dir_path)
        langs[lang] = dir_path

    return langs

def download_mar_for_update_channel_and_platform(config, platform, temp_dir):
    mar = os.environ.get('MAR', 'mar')
    base_url = "http://updater.libreofficecrash.org/update/partial-targets/1/"
    url = base_url + platform + "/" + config.channel
    r = requests.get(url)
    if r.status_code is not 200:
        print(r.content)
        raise Exception("download failed")

    update_info = json.loads(r.content.decode("utf-8"))
    update_files = update_info['updates']
    downloaded_updates = {}
    for update_file in update_files:
        build = update_file["build"]
        filedir = temp_dir + build

        mkdir_p(filedir)

        filepath = filedir + "/complete.mar"
        url = update_file["update"]["url"]
        expected_hash = update_file["update"]["hash"]
        download_file(filepath, url, expected_hash)

        dir_path = os.path.join(filedir, "complete")
        mkdir_p(dir_path)
        extract_mar(filepath, dir_path)

        downloaded_updates[build] = {"complete": dir_path}

        langs = handle_language(update_file["languages"], filedir)
        downloaded_updates[build]["languages"] = langs

    return downloaded_updates

def generate_file_name(current_build_id, old_build_id, mar_name_prefix):
    name = "%s_from_%s_partial.mar" %(mar_name_prefix, old_build_id)
    return name

def generate_lang_file_name(current_build_id, old_build_id, mar_name_prefix, lang):
    name = "%s_%s_from_%s_partial.mar" %(mar_name_prefix, lang, old_build_id)
    return name

def add_single_dir(path):
    dir_name =  [os.path.join(path, name) for name in os.listdir(path) if os.path.isdir(os.path.join(path, name))]
    return dir_name[0]

def main():
    product_name = sys.argv[1]
    workdir = sys.argv[2]
    update_dir = sys.argv[3]
    temp_dir = sys.argv[4]
    mar_name_prefix = sys.argv[5]
    update_config = sys.argv[6]
    platform = sys.argv[7]
    current_build_path = sys.argv[8]
    build_id = sys.argv[9]
    mar_dir = sys.argv[10]

    current_build_path = add_single_dir(current_build_path)

    config = parse_config(update_config)

    updates = download_mar_for_update_channel_and_platform(config, platform, temp_dir)

    data = {"partials": []}

    for build, update in updates.items():
        file_name = generate_file_name(build_id, build, mar_name_prefix)
        mar_file = os.path.join(update_dir, file_name)
        subprocess.call([os.path.join(current_dir_path, 'make_incremental_update.sh'), mar_file, update["complete"], current_build_path])
        sign_mar_file(update_dir, config, mar_file, mar_name_prefix)

        partial_info = {"complete":get_file_info(mar_file, config.base_url), "from": build, "to": build_id, "languages": {}}
        for lang, lang_info in update["languages"].items():
            lang_name = generate_lang_file_name(build_id, build, mar_name_prefix, lang)

            # write the file into the final directory
            lang_mar_file = os.path.join(update_dir, lang_name)

            # the directory of the old language file is of the form
            # workdir/mar/language/en-US/LibreOffice_<version>_<os>_archive_langpack_<lang>/
            language_dir = add_single_dir(os.path.join(mar_dir, "language", lang))
            subprocess.call([os.path.join(current_dir_path, 'make_incremental_update.sh'), lang_mar_file, lang_info, language_dir])
            sign_mar_file(update_dir, config, lang_mar_file, mar_name_prefix)

            # add the partial language info
            partial_info["languages"][lang] = get_file_info(lang_mar_file, config.base_url)

        data["partials"].append(partial_info)

    with open(os.path.join(update_dir, "partial_update_info.json"), "w") as f:
        json.dump(data, f)


if __name__ == '__main__':
    main()
