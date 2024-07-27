import os
import sys
from shutil import copytree, copy2, move, rmtree
import json
import createmsi

build_dir = sys.argv[1]
src_dir = sys.argv[2]
creator_dir = os.path.join(build_dir, 'workdir/installation/MSICreatorLO')

def prepare_project_dir():
    instdir = os.path.join(build_dir, 'instdir')
    fonts_dir = os.path.join(instdir, 'share/fonts')
    new_fonts_dir = os.path.join(creator_dir, 'libo-fonts/share/fonts')
    main_dir = os.path.join(creator_dir, 'main')
    src_uninstaller_icon = os.path.join(src_dir, 'icon-themes/colibre/res/mainapp_48_8.png')
    src_ui_banner = os.path.join(src_dir, 'instsetoo_native/inc_common/windows/msi_templates/Binary/Banner.bmp')
    src_ui_background = os.path.join(src_dir, 'instsetoo_native/inc_common/windows/msi_templates/Binary/Image_2.bmp')
    graphics_dir = os.path.join(creator_dir, 'graphics')
    sdk_dir = os.path.join(creator_dir, 'main/sdk')
    try:
        move(fonts_dir, new_fonts_dir)
        copytree(instdir, main_dir, dirs_exist_ok=True)
        copy2(src_uninstaller_icon, creator_dir)
        os.mkdir(graphics_dir)
        copy2(src_ui_banner, graphics_dir)
        copy2(src_ui_background, graphics_dir)
        rmtree(sdk_dir)
    except FileExistsError as err:
        print(err)

def create_creator_json():
    lo_version = sys.argv[3]
    lo_name = sys.argv[4]
    uninstaller_icon = 'mainapp_48_8.png'
    lo_dictionary = {
        "upgrade_guid": "6f05ed48-a735-4155-ab60-e4cc98455262",
        "version": lo_version,
        "product_name": lo_name,
        "manufacturer": "The Document Foundation",
        "name": lo_name,
        "name_base": lo_name,
        "comments": "Testing a libo installer",
        "installdir": "LibreOffice Test",
        "startmenu_shortcut": "program/soffice.exe",
        "desktop_shortcut": "program/soffice.exe",
        "addremove_icon": uninstaller_icon,
        "major_upgrade": {
            "AllowDowngrades": "yes",
            "IgnoreRemoveFailure": "yes"
        },
        "graphics": {
            "banner": "graphics/Banner.bmp",
            "background": "graphics/Image_2.bmp"
        },
        "parts": [
            {
                "id": "libreoffice",
                "title": "The LibreOffice Suite",
                "description": "This is a test for the LibreOffice installer",
                "absent": "disallow",
                "staged_dir": "main"
            },
            {
                "id": "libreofficefonts",
                "title": "The LibreOffice Fonts ",
                "description": "This is a test for the LibreOffice Fonts",
                "absent": "allow",
                "staged_dir": "libo-fonts"
            }
        ]
    }

    lo_object = json.dumps(lo_dictionary, indent=4)
    with open(os.path.join(creator_dir, 'lo.json'), 'w') as lo_json:
        lo_json.write(lo_object)

def generate_installer():
    os.chdir(creator_dir)
    createmsi.run(['lo.json'])

if __name__ == '__main__':
    prepare_project_dir()
    create_creator_json()
    generate_installer()
