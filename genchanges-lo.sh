#!/bin/bash -e
set -x

DISTRO="tramp"
SOURCEDIR="$1"
TARGETDIR="$2"

STEP=0
function step
{
	echo
	echo "$STEP. $1"
	let "STEP=STEP+1"
}

step "Do prerequisite tests"
if [ ! -d sdk -o ! -d main ]; then
        echo "You have to run this script from the LO deb directory"
        exit 1
fi

##############
# Removing upload
step "Removing old 'upload/'"
rm -rf upload/

function gen_templates {
	if [ "x" = "x${DEBFILES}" ]; then
		echo "No debian packages found for upload!"
		exit 1
	fi

	##############
	# Copy templates
	step "Creating fake templates (control, changelog, files)..."
	mkdir -p upload/debian

	cat > upload/debian/changelog << EOF
$SOURCE ($DEBVER) $DISTRO; urgency=low

  * Faked changelog for LO original build

 -- ${MAINTAINER}  $(date -R)
EOF

	cat > upload/debian/control << EOF
Source: ${SOURCE}
Section: ${SECTION}
Priority: ${PRIORITY}
Maintainer: ${MAINTAINER}

EOF

	BINARIES=""
	for FILE in $DEBFILES
	do
		BASEFILE=`basename $FILE`
		BINARIES="${BINARIES} ${BASEFILE%%_*}"
		echo -n "$BASEFILE ${SECTION} ${PRIORITY}" >> upload/debian/files
		echo >> upload/debian/files
	done

	##############
	# Create "upload/" links
	step "Linking packages into 'upload/' and expanding control"
	for FILE in $DEBFILES; do
		echo "  ${FILE}..."
        	ln -s ../$FILE upload/
		dpkg-deb -f ${FILE} Package Architecture Description >> upload/debian/control
		echo >> upload/debian/control
	done

	cd upload

	##############
	# Generating changes file
	CHG_FILE="${SOURCE}_${DEBVER}.changes"
	step "Generating 'upload/$CHG_FILE'"
	dpkg-genchanges -b -u. > ${CHG_FILE}

	rm -rf debian/

	cd - 1>/dev/null
}

cd "$SOURCEDIR"

DEBVER=$(ls main/libreoffice*-base_*.deb | awk -F '_' '{ print $2; }')
if [ "x" = "x${DEBVER}" ]; then
	echo "Couldn't extract debian version from .deb file"
	exit 1
fi

SECTION="lhm/editors"
PRIORITY="optional"
SOURCE="lo-orig"
MAINTAINER="Linux Client Team <basisclient.it.dir@muenchen.de>"
DEBFILES=$(ls -1 ./main/*${DEBVER}*.deb ./sdk/*${DEBVER}*.deb | tr "\n" " ")

gen_templates

DEBVER=$(ls main/libreoffice*-debian-menus_*.deb | awk -F '_' '{ print $2; }')
SOURCE=$(basename $(ls main/libreoffice*-debian-menus_*.deb) | awk -F '_' '{ print $1; }')
DEBFILES=$(ls -1 ./main/libreoffice*-debian-menus_*.deb)

gen_templates

cd ..
mv "$SOURCEDIR/upload" "$TARGETDIR"

exit 0

