#!/usr/bin/env python3

import sys
import os
import subprocess
import json

from tools import uncompress_file_to_dir, get_file_info, make_complete_mar_name
from config import parse_config
from signing import sign_mar_file
from path import UpdaterPath

current_dir_path = os.path.dirname(os.path.realpath(__file__))

def ensure_dir_exist()

def main():
    if len(sys.argv) < 5:
        print("Usage: create_full_mar_for_languages.py $PRODUCTNAME $WORKDIR $FILENAMEPREFIX $UPDATE_CONFIG")
        sys.exit(1)

    update_config = sys.argv[4]
    filename_prefix = sys.argv[3]
    workdir = sys.argv[2]
    product_name = sys.argv[1]

    if len(update_config) == 0:
        print("missing update config")
        sys.exit(1)

    update_path = UpdaterPath(workdir)
    update_path.ensure_dir_exist()

    target_dir = update_path.get_update_dir()
    temp_dir = update_path.get_current_build_dir()

    config = parse_config(update_config)

    tar_dir = os.path.join(workdir, "installation", product_name, "archive", "install", "en-US")
    tar_file = os.path.join(tar_dir, os.listdir(tar_dir)[0])

    uncompress_dir = uncompress_file_to_dir(tar_file, temp_dir)

    mar_file = make_complete_mar_name(target_dir, filename_prefix)
    subprocess.call([os.path.join(current_dir_path, 'make_full_update.sh'), mar_file, uncompress_dir])


    sign_mar_file(target_dir, config, mar_file, filename_prefix)

    file_info = { 'complete' : get_file_info(mar_file, config.base_url) }

    with open(os.path.join(target_dir, 'complete_info.json'), "w") as complete_info_file:
        json.dump(file_info, complete_info_file, indent = 4)

if __name__ == '__main__':
    main()
