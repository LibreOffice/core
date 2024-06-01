#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

# Complete list of all supported localizations (ISO codes)
completelangiso=af \
am \
ar \
as \
ast \
be \
bg \
bn \
bn-IN \
bo \
br \
brx \
bs \
ca \
ca-valencia \
ckb \
cs \
cy \
da \
de \
dgo \
dsb \
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
fur \
fy \
ga \
gd \
gl \
gu \
gug \
he \
hsb \
hi \
hr \
hu \
hy \
id \
is \
it \
ja \
ka \
kab \
kk \
km \
kmr-Latn \
kn \
ko \
kok \
ks \
lb \
lo \
lt \
lv \
mai \
mk \
ml \
mn \
mni \
mr \
my \
nb \
ne \
nl \
nn \
nr \
nso \
oc \
om \
or \
pa-IN \
pl \
pt \
pt-BR \
ro \
ru \
rw \
sa-IN \
sat \
sd \
sr-Latn \
si \
sid \
sk \
sl \
sq \
sr \
ss \
st \
sv \
sw-TZ \
szl \
ta \
te \
tg \
th \
tn \
tr \
ts \
tt \
ug \
uk \
uz \
ve \
vec \
vi \
xh \
zh-CN \
zh-TW \
zu

# languages with low translation percentage, but still wish to have daily builds
lowcompletion_langs = sun
ifneq ($(ENABLE_RELEASE_BUILD),TRUE)
completelangiso += $(lowcompletion_langs)
else
# allow to manually specify even in release config
completelangiso += $(foreach lang,$(WITH_LANG),$(filter $(lang),$(lowcompletion_langs)))
endif

ifneq ($(WITH_LANG),ALL)
gb_WITH_LANG=$(WITH_LANG)
else
# expand ALL based on language list)
gb_WITH_LANG=$(completelangiso)
endif
gb_HELP_LANGS := en-US

ifneq ($(ENABLE_RELEASE_BUILD),TRUE)
ifneq ($(WITH_LANG),)
gb_WITH_LANG += qtz
gb_HELP_LANGS += qtz
endif
endif

gb_TRANS_LANGS = $(filter-out en-US,$(filter-out qtz,$(gb_WITH_LANG)))

gb_HELP_LANGS += \
	$(foreach lang,$(filter-out $(WITH_POOR_HELP_LOCALIZATIONS),$(gb_TRANS_LANGS)),\
		$(if \
			$(and \
				$(wildcard $(SRCDIR)/helpcontent2/source/auxiliary/$(lang)),\
				$(wildcard $(SRCDIR)/translations/source/$(lang)/helpcontent2)),\
			$(lang)))

# Langs that need special handling for registry files. This is done by
# MAKE_LANG_SPECIFIC flag in scp.
gb_CJK_LANGS := $(filter $(gb_WITH_LANG),ja ko zh-CN zh-TW)
gb_CTL_LANGS := $(filter $(gb_WITH_LANG),ar bo dz fa gu he hi km ky-CN lo my ne or pa-IN ta th)
gb_CTLSEQCHECK_LANGS := $(filter $(gb_CTL_LANGS),km lo th)

# vim: set noet ts=4 sw=4:
