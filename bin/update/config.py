import configparser
import os


class Config(object):

    def __init__(self):
        self.certificate_path = None
        self.certificate_name = None
        self.channel = None
        self.base_url = None
        self.upload_url = None
        self.server_url = None


def parse_config(config_file):
    config = configparser.ConfigParser()
    config.read(os.path.expanduser(config_file))

    data = Config()
    updater_data = config['Updater']
    data.base_url = updater_data['base-url']
    data.certificate_name = updater_data['certificate-name']
    data.certificate_path = updater_data['certificate-path']
    data.channel = updater_data['channel']
    data.upload_url = updater_data['upload-url']
    data.server_url = updater_data["ServerURL"]

    return data
