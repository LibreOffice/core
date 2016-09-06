#!/usr/bin/env python3

import sys
import os
import subprocess
import json

from tools import uncompress_file_to_dir, get_file_info
from config import parse_config

current_dir_path = os.path.dirname(os.path.realpath(__file__))

def make_mar_name(target_dir, filename_prefix):
    filename = filename_prefix + "_complete.mar"
    return os.path.join(target_dir, filename)

def main():
    print(sys.argv)
    if len(sys.argv) < 7:
        print("Usage: create_full_mar_for_languages.py $PRODUCTNAME $WORKDIR $TARGETDIR $TEMPDIR $FILENAMEPREFIX $UPDATE_CONFIG")
        sys.exit(1)

    update_config = sys.argv[6]
    filename_prefix = sys.argv[5]
    temp_dir = sys.argv[4]
    target_dir = sys.argv[3]
    workdir = sys.argv[2]
    product_name = sys.argv[1]

    config = parse_config(update_config)

    tar_dir = os.path.join(workdir, "installation", product_name, "archive", "install", "en-US")
    tar_file = os.path.join(tar_dir, os.listdir(tar_dir)[0])

    uncompress_dir = uncompress_file_to_dir(tar_file, temp_dir)

    # on linux we should stip the symbols from the libraries
    if sys.platform.startswith('linux'):
        subprocess.call('strip -g ' + os.path.join(uncompress_dir, 'program/') + '*', shell=True)

    mar_executable = os.environ.get('MAR', 'mar')
    mar_file = make_mar_name(target_dir, filename_prefix)
    subprocess.call([os.path.join(current_dir_path, 'make_full_update.sh'), mar_file, uncompress_dir])

    signed_mar_file = make_mar_name(target_dir, filename_prefix + '_signed')
    subprocess.call([mar_executable, '-C', target_dir, '-d', config.certificate_path, '-n', config.certificate_name, '-s', mar_file, signed_mar_file])

    os.rename(signed_mar_file, mar_file)

    file_info = { 'complete' : get_file_info(mar_file, config.base_url) }

    with open(os.path.join(target_dir, 'complete_info.json'), "w") as complete_info_file:
        json.dump(file_info, complete_info_file, indent = 4)

if __name__ == '__main__':
    main()
