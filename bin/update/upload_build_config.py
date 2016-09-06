#! /usr/bin/env python3

import sys
import os
import configparser
import requests

dir_path = os.path.dirname(os.path.realpath(__file__))

def main(argv):

    updater_config = sys.argv[2]

    config = configparser.ConfigParser()
    config.read(updater_config)

    user = config["Updater"]["User"]
    password = config["Updater"]["Password"]
    base_address = config["Updater"]["ServerURL"]

    login_url = base_address + "accounts/login/"

    session = requests.session()
    r1 = session.get(login_url)
    csrftoken = session.cookies['csrftoken']

    login_data = { 'username': user,'password': password,
            'csrfmiddlewaretoken': csrftoken }
    r1 = session.post(login_url, data=login_data, headers={"Referer": login_url})

    url = base_address + "update/upload/release"

    build_config = os.path.join(sys.argv[1], "build_config.json")
    r = session.post(url, files={'release_config': open(build_config, "r")})
    print(r.content)
    if r.status_code != 200:
        sys.exit(1)

if __name__ == "__main__":
    main(sys.argv)
