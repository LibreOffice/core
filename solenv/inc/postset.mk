#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

# Default ISO code used when no languages specified
# This language will be always included!
defaultlangiso=en-US

# Complete list of all supported ISO codes
completelangiso=af \
ar \
as \
ast \
be \
bo \
bg \
br \
brx \
bn \
bs \
ca \
ca-XV \
cs \
cy \
da \
de \
dgo \
dz \
el \
en-GB \
en-US \
en-ZA \
eo \
es \
et \
eu \
fa \
fi \
fr \
ga \
gl \
gu \
he \
hi \
hr \
hu \
id \
is \
it \
ja \
ka \
kk \
km \
kn \
ko \
kok \
ks \
ku \
lo \
lt \
lv \
mai \
mk \
mn \
mni \
ml \
mr \
my \
ne \
nb \
nl \
nn \
nr \
nso \
om \
oc \
or \
pa-IN \
pl \
pt \
pt-BR \
ro \
ru \
rw \
sat \
sa-IN \
si \
sd \
sq \
sk \
sl \
sh \
sr \
ss \
st \
sv \
sw-TZ \
te \
ta \
th \
tn \
tr \
ts \
tg \
ug \
uk \
uz \
ve \
vi \
xh \
zh-CN \
zh-TW \
zu

alllangiso=$(strip $(defaultlangiso) $(subst,$(defaultlangiso), $(uniq $(subst,ALL,$(completelangiso) $(WITH_LANG)))))
