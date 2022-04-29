#!/bin/sh

# unpack parameters
VERBOSITY=$1; shift
LANG=$(echo $@ | cut -d ',' -f 1)
PRODNAME=$(echo $@ | cut -d ',' -f 2)
EXTENSION=$(echo $@ | cut -d ',' -f 3)
PKGFORMAT=$(echo $@ | cut -d ',' -f 4)

# need to strip?
if [ ${PKGFORMAT} != "archive" ] ; then
	export ENABLE_STRIP=1
fi

# need to hack buildid?
if [ ${PKGFORMAT}${LIBO_VERSION_PATCH} = "deb0" -o ${PKGFORMAT}${LIBO_VERSION_PATCH} = "rpm0" ] ; then
	LIBO_VERSION_PATCH=1
fi

# switch to verbose?
if [ ${VERBOSITY} = "-verbose" ] ; then
	set -x
fi

# call make_installer
${PERL} -w ${SRCDIR}/solenv/bin/make_installer.pl \
	-f ${BUILDDIR}/instsetoo_native/util/openoffice.lst \
	-l ${LANG} \
	-p ${PRODUCTNAME_WITHOUT_SPACES}${PRODNAME} \
	-u ${instsetoo_OUT} \
	-packer ${COMPRESSIONTOOL} \
	-buildid ${LIBO_VERSION_PATCH} \
	${EXTENSION} \
	-format ${PKGFORMAT} \
	${VERBOSITY}

