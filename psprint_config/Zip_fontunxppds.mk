###############################################################
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
###############################################################



$(eval $(call gb_Zip_Zip,fontunxppds,$(SRCDIR)/psprint_config/configuration/ppds))

ifneq ($(WITH_SYSTEM_PPD_DIR),)
$(eval $(call gb_Zip_add_files,fontunxppds,\
	SGENPRT.PS \
	SGENT42.PS \
	GENERIC.PS \
))
else
$(eval $(call gb_Zip_add_files,fontunxppds,\
        ADISTILL.PS \
        APLW3101.PS \
        APLW3201.PS \
        APLW3601.PS \
        APLW6101.PS \
        APLW8101.PS \
        APLW81F1.PS \
        APLWBGR1.PS \
        APLWCOB1.PS \
        APLWCSB1.PS \
        APLWGRF1.PS \
        APLWGRI1.PS \
        APLWGRJ1.PS \
        APLWIIF2.PS \
        APLWIIG2.PS \
        APLWLOT1.PS \
        APL_IIF1.PS \
        APL_IIG1.PS \
        APL_NTR1.PS \
        APNT518_.PS \
        APNTX501.PS \
        APNTXJ1_.PS \
        APPL_230.PS \
        APPL_380.PS \
        APPL_422.PS \
        APTOLLD1.PS \
        APTOLLW1.PS \
        CAECOGR1.PS \
        DC1152_1.PS \
        EFAX3010.PS \
        EFAX4010.PS \
        EFAX5010.PS \
        EFAX7010.PS \
        EFMX3010.PS \
        EFMX5010.PS \
        EFMX7010.PS \
        EFXJK1F3.PS \
        EFXJK2F3.PS \
        EFXJK303.PS \
        EFXJK3F3.PS \
        EP_08001.PS \
        EP_15201.PS \
        EP_30001.PS \
        EP_50001.PS \
        EP_82F21.PS \
        EP_82F51.PS \
        EP_P0701.PS \
        EP_P1201.PS \
        EP_STPH1.PS \
        GENERIC.PS \
        HP1200C1.PS \
        HP2500C1.PS \
        HP3SI52_.PS \
        HP3SIL21.PS \
        HP4M3_V1.PS \
        HP4M6_V1.PS \
        HP4ML_V1.PS \
        HP4MP3_1.PS \
        HP4MP6_1.PS \
        HP4PLUS3.PS \
        HP4PLUS6.PS \
        HP4SI3_1.PS \
        HP4SI6_1.PS \
        HPIID52_.PS \
        HPIII52_.PS \
        HPIIP52_.PS \
        HPLJ3D1_.PS \
        HPLJ3P1_.PS \
        HPLJ4MV1.PS \
        HPLJ5M_4.PS \
        HPLJ5P_1.PS \
        HPLJ5SI1.PS \
        HPLJ5SIM.PS \
        HPLJ__31.PS \
        HP_3D52_.PS \
        HP_3P52_.PS \
        HP__CLJ1.PS \
        KD8650P1.PS \
        KD92P001.PS \
        KDH00001.PS \
        KDH00002.PS \
        L10__425.PS \
        L20__471.PS \
        L20__493.PS \
        L30__471.PS \
        L30__493.PS \
        L333_523.PS \
        L33__523.PS \
        L50__493.PS \
        L533_523.PS \
        L53__523.PS \
        LEX4039P.PS \
        LEX4079P.PS \
        LEXOPTR_.PS \
        LHCFLRJ5.PS \
        LHCOFLC5.PS \
        LHCOFLR5.PS \
        LHGUBEH3.PS \
        LHGUBHJ4.PS \
        LHHERHJ4.PS \
        LHHERIJ4.PS \
        LHHERKH3.PS \
        LHHERKI4.PS \
        LHHKPHJ7.PS \
        LHHKPIJ7.PS \
        LHHRKPH7.PS \
        LHHRKPI7.PS \
        LHQUSHJ4.PS \
        LHQUSIJ4.PS \
        LHQUSRH3.PS \
        LHQUSRI3.PS \
        LHSIGHJ4.PS \
        LHSIGIJ4.PS \
        LHSIGNH3.PS \
        LHSIGNI3.PS \
        LOPC1200.PS \
        LOPC40.PS \
        LOPC45.PS \
        LOPE310.PS \
        LOPM410.PS \
        LOPS1255.PS \
        LOPS1625.PS \
        LOPS1855.PS \
        LOPS2455.PS \
        LOPS3455.PS \
        LOPT610.PS \
        LOPT612.PS \
        LOPT614.PS \
        LOPT616.PS \
        LOPTRAC.PS \
        LOPTRAEP.PS \
        LOPTRAK.PS \
        LOPTRAN.PS \
        LOPTRAP.PS \
        LOPTRAS.PS \
        LOPTRASC.PS \
        LOPW810.PS \
        LOS1255P.PS \
        LOS1625P.PS \
        LOS1855P.PS \
        LOS2455P.PS \
        LOS3455P.PS \
        LWNT_518.PS \
        SGENPRT.PS \
        SGENT42.PS \
        SNSPREC1.PS \
        TK300PJ1.PS \
        TK300XJ1.PS \
        TK350171.PS \
        TK350172.PS \
        TK350691.PS \
        TK350692.PS \
        TK350PJ2.PS \
        TK360J21.PS \
        TK360J51.PS \
        TK360J71.PS \
        TK450PJ1.PS \
        TK550171.PS \
        TK550391.PS \
        TK550PJ1.PS \
        TKP350J2.PS \
        TKP360P1.PS \
        TKP380J1.PS \
        TKP380P1.PS \
        TKP450J1.PS \
        TKP450P1.PS \
        TKP550J1.PS \
        TKP600J1.PS \
        TKPH3801.PS \
        TKPH4501.PS \
        TKPH6001.PS \
        XRD61354.PS \
        XRD61801.PS \
        XRN17000.PS \
        XRN24320.PS \
        XR_DC230.PS \
))
endif

# vim: set noet sw=4 ts=4:
