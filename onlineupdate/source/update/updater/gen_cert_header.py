#!/usr/bin/env python

from __future__ import print_function
import os
import sys
import binascii

try:
    from configparser import ConfigParser
except ImportError:
    from ConfigParser import SafeConfigParser as ConfigParser

def file_byte_generator(filename):
    with open(filename, "rb") as f:
        block = f.read()
        return block

def eprint(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)

def create_header(array_name, in_filename):
    if sys.version_info >= (3,0):
        hexified = ["0x" + binascii.hexlify(bytes([inp])).decode('ascii') for inp in file_byte_generator(in_filename)]
    else:
        hexified = ["0x" + binascii.hexlify(inp).decode('ascii') for inp in file_byte_generator(in_filename)]
    print("const uint8_t " + array_name + "[] = {")
    print(", ".join(hexified))
    print("};")
    return 0

if __name__ == '__main__':
    if len(sys.argv) < 3:
        eprint('ERROR: usage: gen_cert_header.py array_name update_config_file')
        sys.exit(1)

    if not os.path.exists(sys.argv[2]):
        eprint('The config file %s does not exist'%(sys.argv[2]))
        sys.exit(1)

    config = ConfigParser()
    config.read(sys.argv[2])
    sys.exit(create_header(sys.argv[1], config.get('Updater', 'certificate-der')))
