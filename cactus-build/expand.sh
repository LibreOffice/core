#!/bin/bash
cd ../workdir/installation/LibreOfficeDev/archive/install/en-US/
tar -xzf LibreOff*.tar.gz

mv LibreOffice*archive instdir
cd instdir
rm -r -f help
rm -r -f readmes
rm -r -f libreofficedev
TOTAL_SIZE=`du -s --apparent-size . | awk '{print $1}'`
SHARE_SIZE=`du -s --apparent-size share | awk '{print $1}'`
PROGRAM_SIZE=`du -s --apparent-size program | awk '{print $1}'`
echo
echo total is $TOTAL_SIZE of which share size is $SHARE_SIZE program size is $PROGRAM_SIZE
echo Removing libraries from share
cd share
rm -v -r -f  Scripts
rm -v -r -f  autocorr
rm -v -r -f  autotext
rm -v -r -f  calc
rm -v -r -f  fingerprint
rm -v -r -f  firebird
rm -v -r -f  gallery
rm -v -r -f  libreofficekit
rm -v -r -f  pallette
rm -v -r -f  template
rm -v -r -f  theme_definitions
rm -v -r -f  tipoftheday
rm -r -f  toolbarmode
rm -v -r -f  wizards
rm -v -r -f  wordbook
rm -v -r -f  xdg
rm -v -r -f  xpdfimport
rm -v -r -f  xslt
rm -v -r -f  dtd
rm -v -r -f extensions
rm -v config/images*

echo Removing libraries from program
cd ..
cd program
rm -v JREProperties.class
rm -v -r -f __pycache__
rm -v -r -f classes
rm -v *.png
rm -v libjava*
rm -v libldap*
rm -v libofficebean*
rm -v mailmerge*
rm -v msgbox*
rm -v officehelp*
rm -v -r -f wizards
rm -v xpdfimport
rm -v -r -f opencl*
rm -v -r -f shell
rm -v java*
rm -v libC*
rm -v libEng*
rm -v libLang*
rm -v libOsi*
rm -v libpython3.8.so.1.0
rm -v libpython3.8.so
rm -v libmwaw-0.3-lo.so.3
##rm -v libsdlo.so
rm -v liblocaledata_others.so
rm -v libpostgresql-sdbc-impllo.so
#rm -v libooxlo.so
#rm -v libsclo.so
#rm -v libswlo.so
rm -v libdbulo.so
rm -v -r -f python-core-3.8.16
#rm -v libclucene.so
rm -v libwps-0.4-lo.so.4
rm -v libvbaswobjlo.so
rm -v libvbaobjlo.so
rm -v libwpftdrawlo.so
rm -v libdbalo.so
#rm -v libicuuc.so.72
#rm -v libicui18n.so.72
rm -v libchartcorelo.so
#rm -v libicudata.so.72
#rm -v libmergedlo.so
cd ..
cd ..
# This first off run needs to be done, it configures
# some things that modify data in the file structure
# If this was not done here then the first run
# after installing this data would abort without
# doing anything useful (except the configuration)
# it may be better to do this after installation
# as an installation uuid is created, doing it here
# means that all installation from this program
# will have the same identifier, for our purposes
# this is unlikely to matter.
echo "Doing config run"
instdir/program/soffice.bin \
        --headless \
        --norestore \
        --invisible \
        --nodefault \
        --nofirststartwizard \
        --nolockcheck \
        --nologo \
        --convert-to "html" \
        x.csv

echo
TOTAL_SIZE=`du -s --apparent-size instdir | awk '{print $1}'`
SHARE_SIZE=`du -s --apparent-size instdir/share | awk '{print $1}'`
PROGRAM_SIZE=`du -s --apparent-size instdir/program | awk '{print $1}'`
echo
echo total is $TOTAL_SIZE of which share size is $SHARE_SIZE program size is $PROGRAM_SIZE
echo
echo
echo

