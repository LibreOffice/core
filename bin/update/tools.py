import os
import hashlib
import zipfile
import tarfile


def uncompress_file_to_dir(compressed_file, uncompress_dir):
    extension = os.path.splitext(compressed_file)[1]

    os.makedirs(uncompress_dir, exist_ok=True)

    if extension == '.gz':
        with tarfile.open(compressed_file) as tar:
            tar.extractall(uncompress_dir)
    elif extension == '.zip':
        with zipfile.ZipFile(compressed_file) as zip_file:
            zip_file.extractall(uncompress_dir)

        uncompress_dir = os.path.join(uncompress_dir, os.listdir(uncompress_dir)[0])
        if " " in os.listdir(uncompress_dir)[0]:
            print("replacing whitespace in directory name")
            os.rename(os.path.join(uncompress_dir, os.listdir(uncompress_dir)[0]),
                      os.path.join(uncompress_dir, os.listdir(uncompress_dir)[0].replace(" ", "_")))
    else:
        print("Error: unknown extension " + extension)

    return os.path.join(uncompress_dir, os.listdir(uncompress_dir)[0])


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
            'hashFunction': 'sha512',
            'size': filesize,
            'url': url + os.path.basename(mar_file)}

    return data


def replace_variables_in_string(string, **kwargs):
    new_string = string
    for key, val in kwargs.items():
        new_string = new_string.replace('$(%s)' % key, val)

    return new_string


def make_complete_mar_name(target_dir, filename_prefix):
    filename = filename_prefix + "_complete.mar"
    return os.path.join(target_dir, filename)
