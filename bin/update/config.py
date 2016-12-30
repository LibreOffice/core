
import configparser

class Config(object):

    def __init__(self):
        self.certificate_path = None
        self.certificate_name = None
        self.channel = None
        self.base_url = None
        self.upload_url = None

def parse_config(config_file):
    config = configparser.ConfigParser()
    config.read(config_file)

    data = Config()
    updater_data = config['Updater']
    data.base_url = updater_data['base-url']
    data.certificate_name = updater_data['certificate-name']
    data.certificate_path = updater_data['certificate-path']
    data.channel = updater_data['channel']
    data.upload_url = updater_data['upload-url']

    return data
