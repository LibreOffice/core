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
    echo "Usage:" $0 "<pkg-source-dir> <inst-destination-dir> [-l]"
    echo "    <pkg-source-dir>:       directory *only* containing the Solaris pkg packages to be installed"
    echo "    <inst-destination-dir>: absolute path to where the office and the pkg database will get installed"
    echo "    -l:                     optional parameter to create a link \"soffice\" in $HOME"
    echo
    exit 2
}

cannot_install()
{
    echo
    printf "ERROR:\tCannot install to directory $MY_ROOT\n"
    printf "\t$1\n"
    printf "\tPlease check/cleanup $MY_ROOT or choose a different directory\n"
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
    error "This script is for installation without administrative rights only\n\tPlease use pkgadd to install as root"
fi

#
# Evaluate command line arguments
#

PACKAGE_PATH=$1
MY_ROOT=$2

LINK="nolink"
if [ $# -eq 3 ]
then
  LINK=$3
fi

if [ ! -d $PACKAGE_PATH ]
then
    error "Directory $PACKAGE_PATH does not exist"
fi

if [ ! "${MY_ROOT:0:1}" = "/" ]
then
    error "Invalid installation directory $MY_ROOT, has to be an absolute path"
fi

#
# the admin file is expected to be in the same directory as this script
#

DIRECTORY=`dirname $0`
ADMINFILE=$DIRECTORY/admin
if [ ! -r $ADMINFILE ]
then
    error "Admin file $ADMINFILE does not exist or is not readable"
fi

#
# Check and get the list of packages to install
#

COREPKG=`find $PACKAGE_PATH/* -type d -prune -name "*-core*" -print`
PKGLIST=`find $PACKAGE_PATH/* -type d -prune ! -name "*adabas*" ! -name "*j3*" ! -name "*-desktop-int*" ! -name "*-cde*" ! -name "*-core*" ! -name "*-gnome*" -print`

if [ -z "$COREPKG" ]
then
    error "No core package found in directory $PACKAGE_PATH"
fi

# Do not install gnome-integration package on systems without GNOME
pkginfo -q SUNWgnome-vfs
if [ $? -eq 0 ]
then
  GNOMEPKG=`find $PACKAGE_PATH/* -type d -prune -name "*-gnome*" -print`
fi

echo "Packages found:"
for i in $COREPKG $PKGLIST $GNOMEPKG; do
  echo `basename $i`
done

#
# Check/Create installation directory
#

CORENAME=`basename $COREPKG`
INSTALLDIR=$MY_ROOT`pkgparam -d $PACKAGE_PATH $CORENAME BASEDIR`

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

if [ -d $MY_ROOT/var/sadm ]
then
    cannot_install "pkg database does already exist $MY_ROOT/var/sadm"
fi

# Create the installation directory (otherwise the user would be asked)
mkdir -p $INSTALLDIR
if [ ! -d $INSTALLDIR ]
then
    cannot_install "Unable to create directory $INSTALLDIR"
fi

#
# the tail of the script contains a shared object for overloading the getuid() call
#

GETUID_SO=/tmp/getuid.so.$$
linenum=???
tail +$linenum $0 > $GETUID_SO
if [ ! -r $GETUID_SO ]
then
    error "Cannot create getuid wrapper library $GETUID_SO"
fi

#
# Perform the installation
#

echo "####################################################################"
echo "#     Installation of the found packages                           #"
echo "####################################################################"
echo
echo "Path to the packages       : " $PACKAGE_PATH
echo "Path to the installation   : " $MY_ROOT

export LD_PRELOAD=$GETUID_SO

for i in $COREPKG $PKGLIST $GNOMEPKG; do
  echo /usr/sbin/pkgadd -a $ADMINFILE -d $PACKAGE_PATH -R $MY_ROOT `basename $i`
  /usr/sbin/pkgadd -a $ADMINFILE -d $PACKAGE_PATH -R $MY_ROOT `basename $i`
done

export -n LD_PRELOAD
rm -f $GETUID_SO

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
