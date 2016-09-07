#! /usr/bin/env python3

import sys
import os
import subprocess

from config import parse_config

from tools import replace_variables_in_string

def main():
    product_name = sys.argv[1]
    buildid = sys.argv[2]
    platform = sys.argv[3]
    update_dir = sys.argv[4]
    update_config = sys.argv[5]

    config = parse_config(update_config)
    upload_url = replace_variables_in_string(config.upload_url, channel=config.channel, buildid=buildid, platform=platform)

    target_url, target_dir = upload_url.split(':')

    command = "ssh %s 'mkdir -p %s'"%(target_url, target_dir)
    print(command)
    subprocess.call(command, shell=True)
    for file in os.listdir(update_dir):
        if file.endswith('.mar'):
            subprocess.call(['scp', os.path.join(update_dir, file), upload_url])

if __name__ == '__main__':
    main()
