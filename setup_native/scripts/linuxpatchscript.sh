#!/bin/sh

MYUID=`id | sed "s/(.*//g" | sed "s/.*=//"`

if [ $MYUID -ne 0 ]
then
    echo You need to have super-user rights to install this language package
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

# Asking for the installation directory

echo
echo "Where do you want to install the patch ? [$PRODUCTINSTALLLOCATION] "
read reply leftover
if [ "x$reply" != "x" ]
then
  PRODUCTINSTALLLOCATION="$reply"
fi

# Unpacking

echo "Installing..."

BASEDIR=`dirname $0`

RPMLIST=""
for i in `ls $BASEDIR/RPMS/*.rpm`
do
  rpm -q `rpm -qp --qf "%{NAME}\n" $i` && RPMLIST="$RPMLIST $i"
done

rpm --prefix $PRODUCTINSTALLLOCATION -U $RPMLIST

echo "Done..."

exit 0
