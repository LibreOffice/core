/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: IBM Corporation
 *
 *  Copyright: 2008 by IBM Corporation
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_LWPARROWSTYLES_HXX
#define INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_LWPARROWSTYLES_HXX

#include <xfilter/xfarrowstyle.hxx>
#include <xfilter/xfstylemanager.hxx>
#include <lwpglobalmgr.hxx>

/**
 * @descr   Register arrow styles that are not directly support by SODC.
 *  There are three kinds: reversed arrow, reversed concave arrow and reversed
 *  line arrow. This function should be called before the three arrows be used.
 */
void RegisteArrowStyles()
{
    XFStyleManager* pXFStyleManager = LwpGlobalMgr::GetInstance()->GetXFStyleManager();
    if (!pXFStyleManager)
        return;
    //reversed arrow:
    std::unique_ptr<XFArrowStyle> pArrowStyle100(new XFArrowStyle());
    pArrowStyle100->SetArrowName(u"arrow100"_ustr);
    pArrowStyle100->SetViewbox(u"0 0 140 200"_ustr);
    pArrowStyle100->SetSVGPath(u"M0 180 L70 0 L140 180 L120 180 L70 30 L20 180 L0 180"_ustr);
    pXFStyleManager->AddStyle(std::move(pArrowStyle100));

    //reversed arrow:
    std::unique_ptr<XFArrowStyle> pArrowStyle1(new XFArrowStyle());
    pArrowStyle1->SetArrowName(u"reverse arrow"_ustr);
    pArrowStyle1->SetViewbox(u"0 0 140 200"_ustr);
    pArrowStyle1->SetSVGPath(u"M0 0 L70 200 L140 0"_ustr);
    pXFStyleManager->AddStyle(std::move(pArrowStyle1));

    //reversed concave arrow
    std::unique_ptr<XFArrowStyle> pArrowStyle2(new XFArrowStyle());
    pArrowStyle2->SetArrowName(u"reverse concave arrow"_ustr);
    pArrowStyle2->SetViewbox(u"0 0 140 200"_ustr);
    pArrowStyle2->SetSVGPath(u"M0 0 L80 200 L160 0 L80 100"_ustr);
    pXFStyleManager->AddStyle(std::move(pArrowStyle2));

    //reversed line arrow
    std::unique_ptr<XFArrowStyle> pArrowStyle3(new XFArrowStyle());
    pArrowStyle3->SetArrowName(u"reverse line arrow"_ustr);
    pArrowStyle3->SetViewbox(u"0 0 140 200"_ustr);
    pArrowStyle3->SetSVGPath(u"M0 0 L70 200 L140 0L110 0 L70 160 L20 0 L0 0"_ustr);
    pXFStyleManager->AddStyle(std::move(pArrowStyle3));
}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
