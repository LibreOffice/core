import os
import hashlib
import tarfile

def uncompress_file_to_dir(compressed_file, uncompress_dir):
    command = None
    extension = os.path.splitext(compressed_file)[1]

    try:
        os.mkdir(uncompress_dir)
    except FileExistsError as e:
        pass

    if extension == '.gz':
        tar = tarfile.open(compressed_file)
        tar.extractall(uncompress_dir)
        tar.close()
    elif extension == '.zip':
        pass
    else:
        print("Error: unknown extension " + extension)

    return os.path.join(uncompress_dir, os.listdir(uncompress_dir)[0])

BUF_SIZE = 1048576

def get_hash(file_path):
    sha512 = hashlib.sha512()
    with open(file_path, 'rb') as f:
        while True:
            data = f.read(BUF_SIZE)
            if not data:
                break
            sha512.update(data)
        return sha512.hexdigest()

def get_file_info(mar_file):
    filesize = os.path.getsize(mar_file)
    data = { 'hash' : get_hash(mar_file),
            'hashFunction' : 'sha512',
            'size' : filesize }

    return data
