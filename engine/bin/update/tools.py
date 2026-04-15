import os
import hashlib
import subprocess

from path import convert_to_native


def uncompress_file_to_dir(compressed_file, uncompress_dir):
    os.makedirs(uncompress_dir, exist_ok=True)

    if subprocess.call([
            'msiexec', '/a', convert_to_native(compressed_file).replace('/', '\\'),
            '/quiet',
            'TARGETDIR=' + convert_to_native(uncompress_dir).replace('/', '\\')]) != 0:
        raise Exception('msiexec failed')

    return uncompress_dir


BUF_SIZE = 1048576


def get_hash(file_path):
    sha512 = hashlib.sha512()
    with open(file_path, 'rb') as f:
        while data := f.read(BUF_SIZE):
            sha512.update(data)
        return sha512.hexdigest()


def get_file_info(mar_file, url):
    filesize = os.path.getsize(mar_file)
    data = {'hash': get_hash(mar_file),
            'hash_function': 'sha512',
            'size': filesize,
            'url': url + os.path.basename(mar_file)}

    return data


def make_complete_mar_name(target_dir, filename_prefix):
    filename = filename_prefix + "_complete.mar"
    return os.path.join(target_dir, filename)
