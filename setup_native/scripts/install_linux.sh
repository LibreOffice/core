#!/bin/bash

error()
{
    echo
    printf "ERROR:\t$1\n"
    echo
    echo "User Mode Installation script for developer and knowledgeable early access tester"
    echo
    echo "This installation method is not intended for use in a production environment!"
    echo "Using this script is unsupported and completely at your own risk"
    echo
    echo "Usage:" $0 "<rpm-source-dir> <office-installation-dir> [-l]"
    echo "    <rpm-source-dir>:       directory *only* containing the Linux rpm packages to be installed"
    echo "    <inst-destination-dir>: directory to where the office will get installed into"
    echo "    -l:                     optional parameter to create a link \"soffice\" in $HOME"
    echo
    exit 2
}

cannot_install()
{
    echo
    printf "ERROR:\tCannot proceed with the installation\n"
    printf "\t$1\n"
    echo
    exit 2
}

#
# expect either two or three arguments
#

if [ \( $# -ne 2 -a $# -ne 3 \) -o -z "$1" -o -z "$2" ]
then
    error "Wrong number of arguments"
fi

#
# this script is for userland not for root
#

if [ $UID -eq 0 ]
then
    error "This script is for installation without administrative rights only\n\tPlease use rpm to install as root"
fi

#
# Evaluate command line arguments
#

PACKAGE_PATH=$1
INSTALLDIR=$2

LINK="nolink"
if [ $# -eq 3 ]
then
  LINK=$3
fi

if [ ! -d $PACKAGE_PATH ]
then
    error "Directory $PACKAGE_PATH does not exist"
fi

if [ ! "${INSTALLDIR:0:1}" = "/" ]
then
    error "Invalid installation directory $INSTALLDIR, has to be an absolute path"
fi

#
# Check and get the list of packages to install
#

RPMLIST=`find $PACKAGE_PATH -type f -name "*.rpm" ! -name "*-core-*" ! -name "*-menus-*" ! -name "adabas*" ! -name "j2re*" ! -name "*-gnome*" -print`
CORERPM=`find $PACKAGE_PATH -type f -name "*.rpm" -name "*-core-*" -print`
PREFIX=`rpm -qlp $CORERPM | head -n1`

if [ -z "$CORERPM" ]
then
    error "No core package found in directory $PACKAGE_PATH"
fi

# Do not install gnome-integration package on systems without GNOME
# check for /bin/rpm first, otherwise the rpm database is most likly empty anyway
if [ -x /bin/rpm ]
then
  /bin/rpm -q gnome-vfs2 >/dev/null
  if [ $? -eq 0 ]
  then
    GNOMERPM=`find $PACKAGE_PATH -type f -name "*.rpm" -name "*-gnome*" -print`
  fi
fi

echo "Packages found:"
for i in $CORERPM $RPMLIST $GNOMERPM; do
  echo `basename $i`
done

#
# Check/Create installation directory
#

# We expect that $INSTALLDIR does not exist, however if it is empty we ignore it
if [ -d $INSTALLDIR ]
then
    # if it is not empty we cannot rm it (might be a permission problem as well)
    rmdir $INSTALLDIR
fi
if [ -d $INSTALLDIR ]
then
    cannot_install "Directory $INSTALLDIR exists and is not empty or cannot be removed"
fi

# We expect that rpm_DB_PATH does not exist, however if it is empty we ignore it
#RPM_DB_PATH=$HOME/.RPM_OFFICEDATABASE
RPM_DB_PATH=$INSTALLDIR/.RPM_OFFICEDATABASE

#
# Perform the installation
#

echo "####################################################################"
echo "#     Installation of the found packages                           #"
echo "####################################################################"
echo
echo "Path to the database:       " $RPM_DB_PATH
echo "Path to the packages:       " $PACKAGE_PATH
echo "Path to the installation:   " $INSTALLDIR

# Creating directories
mkdir -p $RPM_DB_PATH
# XXX why ? XXX
chmod 700 $RPM_DB_PATH
if [ ! -d $RPM_DB_PATH ]
then
    cannot_install "Unable to create directory $RPM_DB_PATH"
fi

# Creating RPM database and initializing
rpm --initdb --dbpath $RPM_DB_PATH
echo "Installing the RPMs"

# inject a second slash to the last path segment to avoid rpm 3 concatination bug
NEWPREFIX=`echo $INSTALLDIR | sed -e 's|\(.*\)\/\(.*\)|\1\/\/\2|'`
rpm --install --nodeps -vh --relocate $PREFIX=$NEWPREFIX --dbpath $RPM_DB_PATH $CORERPM
rpm --install --nodeps -vh --relocate $PREFIX=$NEWPREFIX --dbpath $RPM_DB_PATH $RPMLIST $GNOMERPM

#
# Create a link into the users home directory
#

if [ "$LINK" = "-l" ]
then
  echo
  echo "Creating link from $INSTALLDIR/program/soffice to $HOME/soffice"
  rm -f $HOME/soffice
  ln -s $INSTALLDIR/program/soffice $HOME/soffice
fi

# patch the "bootstraprc" to create a self-containing installation
mv $INSTALLDIR/program/bootstraprc $INSTALLDIR/program/bootstraprc.orig
sed 's/UserInstallation=$SYSUSERCONFIG\/.staroffice8/UserInstallation=$ORIGIN\/..\/UserInstallation/g' $INSTALLDIR/program/bootstraprc.orig > $INSTALLDIR/program/bootstraprc

echo
echo "Installation done ..."

exit 0
