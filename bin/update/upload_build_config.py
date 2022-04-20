#! /usr/bin/env python3

import sys
import os
import configparser
import requests

dir_path = os.path.dirname(os.path.realpath(__file__))


def main(argv):
    updater_config = argv[2]

    config = configparser.ConfigParser()
    config.read(os.path.expanduser(updater_config))

    user = config["Updater"]["User"]
    password = config["Updater"]["Password"]
    base_address = config["Updater"]["ServerURL"]

    login_url = base_address + "accounts/login/"

    session = requests.session()
    session.get(login_url)
    csrftoken = session.cookies['csrftoken']

    login_data = {'username': user, 'password': password,
                  'csrfmiddlewaretoken': csrftoken}
    session.post(login_url, data=login_data, headers={"Referer": login_url})

    url = base_address + "update/upload/release"
    data = {'csrfmiddlewaretoken': csrftoken}

    build_config = os.path.join(argv[1], "build_config.json")
    r = session.post(url, files={'release_config': open(build_config, "r")}, data=data)
    print(r.content)
    if r.status_code != 200:
        sys.exit(1)


if __name__ == "__main__":
    main(sys.argv)
