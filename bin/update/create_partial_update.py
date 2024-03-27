#!/usr/bin/env python3
import glob
import json
import os
import re
import subprocess
import sys

from path import UpdaterPath, convert_to_native
from signing import sign_mar_file
from tools import get_file_info, uncompress_file_to_dir


def generate_file_name(old_build_id, mar_name_prefix):
    name = "%s_from_%s_partial.mar" % (mar_name_prefix, old_build_id)
    return name


def main():
    workdir = sys.argv[1]

    updater_path = UpdaterPath(workdir)
    updater_path.ensure_dir_exist()

    mar_name_prefix = sys.argv[2]
    channel = sys.argv[3]
    certificate_path = sys.argv[4]
    certificate_name = sys.argv[5]
    base_url = sys.argv[6]
    product_name = sys.argv[7]
    version = sys.argv[8]
    old_msi = sys.argv[9]

    old_uncompress_dir = uncompress_file_to_dir(old_msi, updater_path.get_previous_build_dir())
    versionini = os.path.join(old_uncompress_dir, 'program', 'version.ini') #TODO: Linux, macOS
    old_build_id = None
    with open(versionini) as f:
        for l in f:
            m = re.fullmatch('buildid=(.*)', l.rstrip())
            if m:
                old_build_id = m.group(1)
                break
    if old_build_id is None:
        raise Exception(f'Cannot find buildid in {versionini}')

    new_msi_file_glob = os.path.join(updater_path.get_workdir(), "installation", product_name, "msi", "install", "*", f'{product_name}_*.msi')
    new_msi_files = glob.glob(new_msi_file_glob)
    if len(new_msi_files) != 1:
        raise Exception(f'`{new_msi_file_glob}` does not match exactly one file')
    new_msi_file = new_msi_files[0]
    new_uncompress_dir = uncompress_file_to_dir(new_msi_file, updater_path.get_current_build_dir())

    update_dir = updater_path.get_update_dir()

    file_name = generate_file_name(old_build_id, mar_name_prefix)
    mar_file = os.path.join(update_dir, file_name)

    os.putenv('MOZ_PRODUCT_VERSION', version)
    os.putenv('MAR_CHANNEL_ID', 'LOOnlineUpdater')
    subprocess.call([os.path.join(workdir, 'UnpackedTarball/onlineupdate/tools/update-packaging/make_incremental_update.sh'), convert_to_native(mar_file),
                     convert_to_native(old_uncompress_dir), convert_to_native(new_uncompress_dir)])

    sign_mar_file(update_dir, certificate_path, certificate_name, mar_file, mar_name_prefix)

    data = {
        'from': old_build_id,
        'see also': '',
        'update': get_file_info(mar_file, base_url),
        'languages': {}
    }
    with open(os.path.join(update_dir, channel), "w") as f:
        json.dump(data, f, indent=4)


if __name__ == '__main__':
    main()
