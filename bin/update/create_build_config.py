#! /usr/bin/env python3

import json
import sys
import os

from config import parse_config

from tools import replace_variables_in_string

def update_all_url_entries(data, **kwargs):
    data['complete']['url'] = replace_variables_in_string(data['complete']['url'], **kwargs)
    for language in data['languages']:
        language['complete']['url'] = replace_variables_in_string(language['complete']['url'], **kwargs)

def main(argv):
    if len(argv) < 7:
        print("Usage: create_build_config.py $PRODUCTNAME $VERSION $BUILDID $PLATFORM $TARGETDIR $UPDATE_CONFIG")

    config = parse_config(argv[6])

    data = { 'productName' : argv[1],
            'version' : argv[2],
            'buildNumber' : argv[3],
            'updateChannel' : config.channel,
            'platform' : argv[4]
            }

    extra_data_files = ['complete_info.json', 'complete_lang_info.json']

    for extra_file in extra_data_files:
        extra_file_path = os.path.join(argv[5], extra_file)
        with open(extra_file_path, "r") as f:
            extra_data = json.load(f)
            data.update(extra_data)

    update_all_url_entries(data, channel=config.channel, platform=argv[4], buildid=argv[3], version=argv[2])

    with open(os.path.join(argv[5], "build_config.json"), "w") as f:
        json.dump(data, f, indent=4)

if __name__ == "__main__":
    main(sys.argv)
