from tools import make_complete_mar_name

import os
import subprocess
import path


def sign_mar_file(target_dir, config, mar_file, filename_prefix):
    signed_mar_file = make_complete_mar_name(target_dir, filename_prefix + '_signed')
    mar_executable = os.environ.get('MAR', 'mar')
    subprocess.check_call([mar_executable, '-C', path.convert_to_native(target_dir), '-d',
                           path.convert_to_native(config.certificate_path), '-n', config.certificate_name, '-s',
                           path.convert_to_native(mar_file), path.convert_to_native(signed_mar_file)])

    os.rename(signed_mar_file, mar_file)
