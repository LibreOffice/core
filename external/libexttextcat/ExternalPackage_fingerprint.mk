# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,libexttextcat_fingerprint,exttextcat))

$(eval $(call gb_ExternalPackage_set_outdir,libexttextcat_fingerprint,$(INSTROOT)))

$(eval $(call gb_ExternalPackage_add_unpacked_files,libexttextcat_fingerprint,$(LIBO_SHARE_FOLDER)/fingerprint,\
	langclass/fpdb.conf \
	langclass/LM/ab.lm \
	langclass/LM/ace.lm \
	langclass/LM/ada.lm \
	langclass/LM/af.lm \
	langclass/LM/alt.lm \
	langclass/LM/am.lm \
	langclass/LM/ar.lm \
	langclass/LM/arn.lm \
	langclass/LM/ast.lm \
	langclass/LM/ay.lm \
	langclass/LM/az-Cyrl.lm \
	langclass/LM/az.lm \
	langclass/LM/ban.lm \
	langclass/LM/be.lm \
	langclass/LM/bem.lm \
	langclass/LM/bg.lm \
	langclass/LM/bho.lm \
	langclass/LM/bi.lm \
	langclass/LM/bik.lm \
	langclass/LM/bm.lm \
	langclass/LM/bn.lm \
	langclass/LM/bo.lm \
	langclass/LM/br.lm \
	langclass/LM/bs.lm \
	langclass/LM/ca.lm \
	langclass/LM/cs.lm \
	langclass/LM/cy.lm \
	langclass/LM/da.lm \
	langclass/LM/de.lm \
	langclass/LM/dv.lm \
	langclass/LM/ee.lm \
	langclass/LM/el.lm \
	langclass/LM/en.lm \
	langclass/LM/eo.lm \
	langclass/LM/es.lm \
	langclass/LM/et.lm \
	langclass/LM/eu.lm \
	langclass/LM/fa.lm \
	langclass/LM/fi.lm \
	langclass/LM/fj.lm \
	langclass/LM/fo.lm \
	langclass/LM/fr.lm \
	langclass/LM/fur.lm \
	langclass/LM/fy.lm \
	langclass/LM/ga.lm \
	langclass/LM/gd.lm \
	langclass/LM/gl.lm \
	langclass/LM/grc.lm \
	langclass/LM/gu.lm \
	langclass/LM/gug.lm \
	langclass/LM/gv.lm \
	langclass/LM/ha-NG.lm \
	langclass/LM/haw.lm \
	langclass/LM/he.lm \
	langclass/LM/hi.lm \
	langclass/LM/hil.lm \
	langclass/LM/hr.lm \
	langclass/LM/hsb.lm \
	langclass/LM/ht.lm \
	langclass/LM/hu.lm \
	langclass/LM/hy.lm \
	langclass/LM/ia.lm \
	langclass/LM/id.lm \
	langclass/LM/is.lm \
	langclass/LM/it.lm \
	langclass/LM/ja.lm \
	langclass/LM/ka.lm \
	langclass/LM/kk.lm \
	langclass/LM/kl.lm \
	langclass/LM/km.lm \
	langclass/LM/kn.lm \
	langclass/LM/ko.lm \
	langclass/LM/ktu.lm \
	langclass/LM/ky.lm \
	langclass/LM/la.lm \
	langclass/LM/lb.lm \
	langclass/LM/lg.lm \
	langclass/LM/ln.lm \
	langclass/LM/lo.lm \
	langclass/LM/lt.lm \
	langclass/LM/lv.lm \
	langclass/LM/mai.lm \
	langclass/LM/mi.lm \
	langclass/LM/mk.lm \
	langclass/LM/ml.lm \
	langclass/LM/mn.lm \
	langclass/LM/mos.lm \
	langclass/LM/mr.lm \
	langclass/LM/ms.lm \
	langclass/LM/mt.lm \
	langclass/LM/my.lm \
	langclass/LM/nb.lm \
	langclass/LM/ne.lm \
	langclass/LM/nl.lm \
	langclass/LM/nn.lm \
	langclass/LM/nr.lm \
	langclass/LM/nso.lm \
	langclass/LM/ny.lm \
	langclass/LM/oc.lm \
	langclass/LM/om.lm \
	langclass/LM/pa.lm \
	langclass/LM/pl.lm \
	langclass/LM/plt.lm \
	langclass/LM/pt.lm \
	langclass/LM/qxa.lm \
	langclass/LM/rm.lm \
	langclass/LM/ro.lm \
	langclass/LM/ru.lm \
	langclass/LM/rue.lm \
	langclass/LM/rw.lm \
	langclass/LM/sa.lm \
	langclass/LM/sc.lm \
	langclass/LM/sco.lm \
	langclass/LM/sd.lm \
	langclass/LM/se.lm \
	langclass/LM/sg.lm \
	langclass/LM/shs.lm \
	langclass/LM/si.lm \
	langclass/LM/sk.lm \
	langclass/LM/sl.lm \
	langclass/LM/so.lm \
	langclass/LM/sq.lm \
	langclass/LM/sr-Cyrl.lm \
	langclass/LM/sr-Latn.lm \
	langclass/LM/ss.lm \
	langclass/LM/st.lm \
	langclass/LM/sv.lm \
	langclass/LM/sw.lm \
	langclass/LM/ta.lm \
	langclass/LM/tet.lm \
	langclass/LM/tg.lm \
	langclass/LM/th.lm \
	langclass/LM/ti.lm \
	langclass/LM/tk.lm \
	langclass/LM/tl.lm \
	langclass/LM/tn.lm \
	langclass/LM/tpi.lm \
	langclass/LM/tr.lm \
	langclass/LM/ts.lm \
	langclass/LM/tt.lm \
	langclass/LM/ty.lm \
	langclass/LM/ug.lm \
	langclass/LM/uk.lm \
	langclass/LM/ur.lm \
	langclass/LM/uz.lm \
	langclass/LM/ve.lm \
	langclass/LM/vi.lm \
	langclass/LM/wa.lm \
	langclass/LM/xh.lm \
	langclass/LM/yi.lm \
	langclass/LM/yo.lm \
	langclass/LM/zh-Hans.lm \
	langclass/LM/zh-Hant.lm \
	langclass/LM/zu.lm \
))

# vim: set noet sw=4 ts=4:
