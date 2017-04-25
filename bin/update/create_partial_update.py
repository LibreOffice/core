#!/usr/bin/env python3
import requests
import json
import sys
import hashlib
import os
import subprocess
import errno

from config import parse_config

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
        subprocess.call([mar, "-C", dir_path, "-x", lang_file])
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
        subprocess.call([mar, "-C", dir_path, "-x", filepath])

        downloaded_updates[build] = {"complete": dir_path}

        langs = handle_language(update_file["languages"], filedir)
        downloaded_updates[build]["languages"] = langs

    return downloaded_updates

def generate_file_name(current_build_id, old_build_id, mar_name_prefix):
    name = "%s_from_%s_to_%s_partial.mar" %(mar_name_prefix, current_build_id, old_build_id)
    return name

def generate_lang_file_name(current_build_id, old_build_id, mar_name_prefix, lang):
    name = "%s_%s_from_%s_to_%s_partial.mar" %(mar_name_prefix, lang, current_build_id, old_build_id)
    return name

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
    dir_name =  [os.path.join(current_build_path, name) for name in os.listdir(current_build_path) if os.path.isdir(os.path.join(current_build_path, name))]
    current_build_path = dir_name[0]

    config = parse_config(update_config)

    updates = download_mar_for_update_channel_and_platform(config, platform, temp_dir)

    print(updates)
    for build, update in updates.items():
        file_name = generate_file_name(build_id, build, mar_name_prefix)
        mar_file = os.path.join(temp_dir, build, file_name)
        print(mar_file)
        print(current_build_path)
        print(update["complete"])
        subprocess.call([os.path.join(current_dir_path, 'make_incremental_update.sh'), mar_file, current_build_path, update["complete"]])
        for lang in update["languages"].items():
            lang_name = generate_lang_file_name(build_id, build, mar_name_prefix, lang)
            lang_mar_file = os.path.join(temp_dir, build, lang_name)
            #subprocess.call([os.path.join(current_dir_path, 'make_incremental_update.sh'), mar_file, current_build_path, update["complete"]])


if __name__ == '__main__':
    main()
