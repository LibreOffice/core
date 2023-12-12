#! /usr/bin/env python3

import sys
import os
import subprocess

from path import convert_to_unix

from tools import replace_variables_in_string


def main():
    # product_name = sys.argv[1]
    buildid = sys.argv[2]
    platform = sys.argv[3]
    update_dir = sys.argv[4]
    upload_url_arg = sys.argv[5]
    channel = sys.argv[6]

    upload_url = replace_variables_in_string(upload_url_arg, channel=channel, buildid=buildid,
                                             platform=platform)

    target_url, target_dir = upload_url.split(':')

    command = "ssh %s 'mkdir -p %s'" % (target_url, target_dir)
    print(command)
    subprocess.call(command, shell=True)
    for file in os.listdir(update_dir):
        if file.endswith('.mar'):
            subprocess.call(['scp', convert_to_unix(os.path.join(update_dir, file)), upload_url])


if __name__ == '__main__':
    main()
