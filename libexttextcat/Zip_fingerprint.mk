# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Zip_Zip,fingerprint,$(call gb_UnpackedTarball_get_dir,exttextcat)/langclass))

$(eval $(call gb_Zip_use_unpacked,fingerprint,exttextcat))

$(eval $(call gb_Zip_add_files,fingerprint,\
	fpdb.conf \
	LM/ab.lm \
	LM/ace.lm \
	LM/ada.lm \
	LM/af.lm \
	LM/alt.lm \
	LM/am.lm \
	LM/ar.lm \
	LM/arn.lm \
	LM/ast.lm \
	LM/ay.lm \
	LM/az-Cyrl.lm \
	LM/az.lm \
	LM/ban.lm \
	LM/be.lm \
	LM/bem.lm \
	LM/bg.lm \
	LM/bho.lm \
	LM/bik.lm \
	LM/bi.lm \
	LM/bm.lm \
	LM/bn.lm \
	LM/bo.lm \
	LM/br.lm \
	LM/bs.lm \
	LM/ca.lm \
	LM/cs.lm \
	LM/cy.lm \
	LM/da.lm \
	LM/de.lm \
	LM/dv.lm \
	LM/ee.lm \
	LM/el.lm \
	LM/en.lm \
	LM/eo.lm \
	LM/es.lm \
	LM/et.lm \
	LM/eu.lm \
	LM/fa.lm \
	LM/fi.lm \
	LM/fj.lm \
	LM/fo.lm \
	LM/fr.lm \
	LM/fur.lm \
	LM/fy.lm \
	LM/ga.lm \
	LM/gd.lm \
	LM/gl.lm \
	LM/grc.lm \
	LM/gug.lm \
	LM/gu.lm \
	LM/gv.lm \
	LM/ha-NG.lm \
	LM/haw.lm \
	LM/he.lm \
	LM/hil.lm \
	LM/hi.lm \
	LM/hr.lm \
	LM/hsb.lm \
	LM/ht.lm \
	LM/hu.lm \
	LM/hy.lm \
	LM/ia.lm \
	LM/id.lm \
	LM/is.lm \
	LM/it.lm \
	LM/ja.lm \
	LM/ka.lm \
	LM/kk.lm \
	LM/kl.lm \
	LM/km.lm \
	LM/kn.lm \
	LM/ko.lm \
	LM/ktu.lm \
	LM/ky.lm \
	LM/la.lm \
	LM/lb.lm \
	LM/lg.lm \
	LM/ln.lm \
	LM/lo.lm \
	LM/lt.lm \
	LM/lv.lm \
	LM/mai.lm \
	LM/mi.lm \
	LM/mk.lm \
	LM/ml.lm \
	LM/mn.lm \
	LM/mos.lm \
	LM/mr.lm \
	LM/ms.lm \
	LM/mt.lm \
	LM/my.lm \
	LM/nb.lm \
	LM/ne.lm \
	LM/nl.lm \
	LM/nn.lm \
	LM/nr.lm \
	LM/nso.lm \
	LM/ny.lm \
	LM/oc.lm \
	LM/om.lm \
	LM/pa.lm \
	LM/pl.lm \
	LM/plt.lm \
	LM/pt.lm \
	LM/qxa.lm \
	LM/rm.lm \
	LM/ro.lm \
	LM/rue.lm \
	LM/ru.lm \
	LM/rw.lm \
	LM/sa.lm \
	LM/sc.lm \
	LM/sco.lm \
	LM/sd.lm \
	LM/se.lm \
	LM/sg.lm \
	LM/shs.lm \
	LM/si.lm \
	LM/sk.lm \
	LM/sl.lm \
	LM/so.lm \
	LM/sq.lm \
	LM/sr-Cyrl.lm \
	LM/sr-Latn.lm \
	LM/ss.lm \
	LM/st.lm \
	LM/sv.lm \
	LM/sw.lm \
	LM/ta.lm \
	LM/tet.lm \
	LM/tg.lm \
	LM/th.lm \
	LM/ti.lm \
	LM/tk.lm \
	LM/tl.lm \
	LM/tn.lm \
	LM/tpi.lm \
	LM/tr.lm \
	LM/ts.lm \
	LM/tt.lm \
	LM/ty.lm \
	LM/ug.lm \
	LM/uk.lm \
	LM/ur.lm \
	LM/uz.lm \
	LM/ve.lm \
	LM/vi.lm \
	LM/wa.lm \
	LM/xh.lm \
	LM/yi.lm \
	LM/yo.lm \
	LM/zh-Hans.lm \
	LM/zh-Hant.lm \
	LM/zu.lm \
))

$(eval $(call gb_Zip_add_commandoptions,fingerprint,-j))

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
