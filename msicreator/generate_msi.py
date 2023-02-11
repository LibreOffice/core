import os, json, subprocess, shutil
from pathlib import Path
from shutil import copytree

source_dir = Path(os.getcwd())
msicreator_path = Path(source_dir) / 'msicreator'
gen_installer =  Path(source_dir) / 'workdir/installation/LibreOfficeDev/msi/install/en-US'


def prepare_msicreator_dir():
    instdir = source_dir / 'instdir'
    new_instdir = msicreator_path / 'instdir'
    o_fonts_dir = new_instdir / 'share/fonts'
    n_fonts_dir = msicreator_path / 'fontinst/share/fonts'
    n_fonts_dir.mkdir(parents=True, exist_ok=True)
    graphics = source_dir / 'instsetoo_native/inc_common/windows/msi_templates/Binary'
    msi_graphics = msicreator_path / 'graphics'
    try:
        copytree(instdir, new_instdir, dirs_exist_ok=True)
        shutil.copy2(graphics / 'Banner.bmp', msi_graphics)
        if n_fonts_dir.exists():
            shutil.rmtree(n_fonts_dir)
            shutil.move(str(o_fonts_dir), str(n_fonts_dir))
    except FileExistsError as err:
        print(err)
        return False
    return True


def create_libo_json():
    with open(msicreator_path / 'lo_data.json', 'r') as json_file:
            data = json.load(json_file)
    libo_dictionary = {
        "upgrade_guid": "{EFD2F52B-6C0E-4F84-9E95-79C5F69DF479}",
        "version": f"{data['LIBO_VERSION_MAJOR']}.{data['LIBO_VERSION_MINOR']}.{data['LIBO_VERSION_MICRO']}.{data['LIBO_VERSION_PATCH']}",
        "product_name": data["PACKAGE_NAME"],
        "manufacturer": "The Document Foundation",
        "name": data["PACKAGE_NAME"],
        "name_base": "libreoffice",
        "comments": "LibreOffice",
        "installdir": "LibreOffice Winstaller Test",
        "license_file": "license.rtf",
        "startmenu_shortcut": "program/soffice.exe",
        "desktop_shortcut": "program/soffice.exe",
        "major_upgrade": {
            "AllowDowngrades": "yes",
            "IgnoreRemoveFailure": "yes",
        },
        "graphics": {
            "banner": "graphics/Banner.bmp",
            "background": "graphics/Image.bmp"
        },
        "parts": [
            {
                "id": "LibreOffice",
                "title": "LibreOffice",
                "description": "LibreOffice, the office productivity suite provided by The Document Foundation. See: https://www.documentfoundation.org",
                "absent": "disallow",
                "staged_dir": "instdir",
            },
            {
                "id": "LibreOfficeFonts",
                "title": "LibreOffice Fonts",
                "description": "LibreOffice Fonts",
                "absent": "disallow",
                "staged_dir": "fontinst",
            }
        ]
    }

    try:
        with open(msicreator_path / 'libreoffice.json', 'w') as lo_json:
            json.dump(libo_dictionary, lo_json, indent=4)
    except Exception as exc:
        print(f'Error: {exc}')

def create_libo_msi():
    os.chdir(msicreator_path)
    filename = Path('libreoffice.json')
    output_dir = gen_installer

    if not filename.exists():
        print('ERROR: json file does not exist')
        return
    else:
       create_installer = subprocess.run(f'python createmsi.py {filename}', shell=False, capture_output=True, text=True)

    if create_installer.returncode == 0:
        print(f"stdout: {create_installer.stdout}")
        libo_msi = msicreator_path / 'libreoffice-7.6.0.0-64.msi'
        shutil.move(str(libo_msi), str(output_dir))
    else:
        print(f"ERROR: failed to create an installer {create_installer.returncode}")
        print(f"stderr: {create_installer.stderr}")



if __name__ == '__main__':
    prepare_msicreator_dir()
    create_libo_json()
    create_libo_msi()


