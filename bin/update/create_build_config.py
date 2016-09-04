#! /usr/bin/env python3

import json
import sys
import os

def update_url(old_url, **kwargs):
    new_url = old_url
    for key, val in kwargs.items():
        new_url = new_url.replace(key, val)

    return new_url

def update_all_url_entries(data, **kwargs):
    data['complete']['url'] = update_url(data['complete']['url'], **kwargs)
    for language in data['languages']:
        language['complete']['url'] = update_url(language['complete']['url'], **kwargs)

def main(argv):
    if len(argv) < 7:
        print("Usage: create_build_config.py $PRODUCTNAME $VERSION $BUILDID $UPDATECHANNEL $PLATFORM $TARGETDIR")

    data = { 'productName' : argv[1],
            'version' : argv[2],
            'buildNumber' : argv[3],
            'updateChannel' : argv[4],
            'platform' : argv[5]
            }

    extra_data_files = ['complete_info.json', 'complete_lang_info.json']

    for extra_file in extra_data_files:
        extra_file_path = os.path.join(argv[6], extra_file)
        with open(extra_file_path, "r") as f:
            extra_data = json.load(f)
            data.update(extra_data)

    update_all_url_entries(data, channel=argv[4], platform=argv[5], buildid=argv[3], version=argv[2])

    with open(os.path.join(argv[6], "build_config.json"), "w") as f:
        json.dump(data, f, indent=4)

if __name__ == "__main__":
    main(sys.argv)
