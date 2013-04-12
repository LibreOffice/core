#!/bin/bash
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

topdirs=`find . -mindepth 1 -maxdepth 1 -type d -not -name sal`
mkdir -p include/
for dir in $topdirs
do
    dir=`echo "$dir"| sed -e 's/^..//'`
    if test -d $dir/inc/$dir
    then
        if test -f $dir/Package_inc.mk
        then
            if test -f $dir/Module_$dir.mk
            then
                git mv $dir/inc/$dir include/$dir
                git rm $dir/Package_inc.mk
                grep -v Package_inc $dir/Module_$dir.mk > $dir/Module_dir.mk.new
                mv -f $dir/Module_dir.mk.new $dir/Module_$dir.mk
                git add $dir/Module_$dir.mk
            else
                echo "WARN: no $dir/Module_$dir.mk"
            fi
        else
            echo "WARN: no file $dir/Package_inc.mk"
        fi
    fi
done
#grep -v Package_inc.mk sal/CustomTarget_sal_allheaders.mk > sal/CustomTarget_sal_allheaders.mk.new
#mv sal/CustomTarget_sal_allheaders.mk.new sal/CustomTarget_sal_allheaders.mk
#git add sal/CustomTarget_sal_allheaders.mk

# we like to be special ...
sed -ie 's/\/svtools\/inc\/svtools/\/include\/svtools\//' svtools/Library_svt.mk
sed -ie 's/\/sfx2\/inc\/sfx2/\/include\/sfx2\//' sfx2/Library_sfx.mk
sed -ie 's/\/sfx2\/inc\/sfx2/\/include\/sfx2\//' sfx2/Library_qstart_gtk.mk
git add svtools/Library_svt.mk sfx2/Library_sfx.mk sfx2/Library_qstart_gtk.mk

# urgh
sed -ie 's/\.\.\/svx\//svx\//' svx/source/svdraw/svdoashp.cxx
git add svx/source/svdraw/svdoashp.cxx

# this one is making me speechless ...
echo "// ah, yes: thats a sane thing to do -- deliver the same header _twice_ with different names" > include/toolkit/unohlp.hxx
echo "// please kill me (this file)!" >> include/toolkit/unohlp.hxx
echo "#include <toolkit/helper/vclunohelper.hxx>" >> include/toolkit/unohlp.hxx
git add include/toolkit/unohlp.hxx
