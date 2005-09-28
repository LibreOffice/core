#!/bin/sh

MYUID=`id | sed "s/(.*//g" | sed "s/.*=//"`

if [ $MYUID -ne 0 ]
then
    echo You need to have super-user permissions to run this patch script
    exit 1
fi

echo
echo "Searching for the PRODUCTNAMEPLACEHOLDER installation ..."

RPMNAME=`rpm -qa | grep PRODUCTNAMEPLACEHOLDER-core01`

if [ "x$RPMNAME" != "x" ]
then
  PRODUCTINSTALLLOCATION="`rpm -ql $RPMNAME | head -n 1`"
else
  echo "PRODUCTNAMEPLACEHOLDER is not installed"
  exit 1
fi

# Last chance to exit ..
echo
read -p "Patching the installation in ${PRODUCTINSTALLLOCATION}. Continue (y/n) ? " -n 1 reply leftover
echo
[ "$reply" == "y" ] || exit 1

echo
echo "About to update the following packages ..."

BASEDIR=`dirname $0`

RPMLIST=""
for i in `ls $BASEDIR/RPMS/*.rpm`
do
  rpm --query `rpm --query --queryformat "%{NAME}\n" --package $i` && RPMLIST="$RPMLIST $i"
done

# Save UserInstallation value
BOOTSTRAPRC="${PRODUCTINSTALLLOCATION}/program/bootstraprc"
USERINST=`grep UserInstallation ${BOOTSTRAPRC}`

echo
rpm --upgrade -v --hash --prefix $PRODUCTINSTALLLOCATION --notriggers $RPMLIST
echo

# Some RPM versions have problems with -U and --prefix
if [ ! -f ${BOOTSTRAPRC} ]; then
  echo Update failed due to a bug in RPM, uninstalling ..
  rpm --erase -v --nodeps --notriggers `rpm --query --queryformat "%{NAME} " --package $RPMLIST`
  echo
  echo Now re-installing new packages ..
  echo
  rpm --install -v --hash --prefix $PRODUCTINSTALLLOCATION --notriggers $RPMLIST
  echo
fi

# Restore the UserInstallation key if necessary
DEFUSERINST=`grep UserInstallation ${BOOTSTRAPRC}`
if [ "${USERINST}" != "${DEFUSERINST}" ]; then
  mv -f ${BOOTSTRAPRC} ${BOOTSTRAPRC}.$$
  sed "s|UserInstallation.*|${USERINST}|" ${BOOTSTRAPRC}.$$ > ${BOOTSTRAPRC}
  rm -f ${BOOTSTRAPRC}.$$
fi

echo "Done."

exit 0
