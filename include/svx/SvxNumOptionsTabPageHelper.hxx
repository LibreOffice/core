/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SVX_SVXNUMOPTIONSTABPAGEHELPER_HXX
#define INCLUDED_SVX_SVXNUMOPTIONSTABPAGEHELPER_HXX

#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/RelOrientation.hpp>

#include <tools/mapunit.hxx>
#include <i18nlangtag/mslangid.hxx>
#include <editeng/numitem.hxx>
#include <svl/eitem.hxx>
#include <vcl/layout.hxx>
#include <vcl/svapp.hxx>
#include <svx/gallery.hxx>
#include <svl/urihelper.hxx>
#include <editeng/brushitem.hxx>
#include <svl/intitem.hxx>
#include <sfx2/objsh.hxx>
#include <vcl/graph.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/settings.hxx>
#include <vcl/builderfactory.hxx>
#include <editeng/flstitem.hxx>
#include <svx/dlgutil.hxx>
#include <svx/xtable.hxx>
#include <svx/drawitem.hxx>
#include <svx/numvset.hxx>
#include <sfx2/htmlmode.hxx>
#include <unotools/pathoptions.hxx>
#include <svtools/ctrltool.hxx>
#include <editeng/unolingu.hxx>
#include <com/sun/star/style/NumberingType.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/text/DefaultNumberingProvider.hpp>
#include <com/sun/star/text/XDefaultNumberingProvider.hpp>
#include <com/sun/star/text/XNumberingFormatter.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/text/XNumberingTypeInfo.hpp>
#include <svx/dialmgr.hxx>
#include <svx/dialogs.hrc>

#include "sfx2/opengrf.hxx"

using namespace css;
using namespace css::uno;
using namespace css::beans;
using namespace css::lang;
using namespace css::i18n;
using namespace css::text;
using namespace css::container;
using namespace css::style;

class SVX_DLLPUBLIC SvxNumOptionsTabPageHelper
{
public:
    static Reference<XDefaultNumberingProvider> lcl_GetNumberingProvider()
    {
        Reference<XComponentContext>         xContext( ::comphelper::getProcessComponentContext() );
        Reference<XDefaultNumberingProvider> xRet = text::DefaultNumberingProvider::create(xContext);
        return xRet;
    }

    static void GetI18nNumbering( ListBox& rFmtLB, sal_uInt16 nDoNotRemove )
    {
        Reference<XDefaultNumberingProvider> xDefNum = lcl_GetNumberingProvider();
        Reference<XNumberingTypeInfo> xInfo(xDefNum, UNO_QUERY);

        // Extended numbering schemes present in the resource but not offered by
        // the i18n framework per configuration must be removed from the listbox.
        // Do not remove a special entry matching nDoNotRemove.
        const sal_uInt16 nDontRemove = SAL_MAX_UINT16;
        ::std::vector< sal_uInt16> aRemove( rFmtLB.GetEntryCount(), nDontRemove);
        for (size_t i=0; i<aRemove.size(); ++i)
        {
            sal_uInt16 nEntryData = (sal_uInt16)reinterpret_cast<sal_uLong>(rFmtLB.GetEntryData(
                    sal::static_int_cast< sal_Int32 >(i)));
            if (nEntryData > NumberingType::CHARS_LOWER_LETTER_N && nEntryData != nDoNotRemove)
                aRemove[i] = nEntryData;
        }
        if(xInfo.is())
        {
            Sequence<sal_Int16> aTypes = xInfo->getSupportedNumberingTypes(  );
            const sal_Int16* pTypes = aTypes.getConstArray();
            for(sal_Int32 nType = 0; nType < aTypes.getLength(); nType++)
            {
                sal_Int16 nCurrent = pTypes[nType];
                if(nCurrent > NumberingType::CHARS_LOWER_LETTER_N)
                {
                    bool bInsert = true;
                    for(sal_Int32 nEntry = 0; nEntry < rFmtLB.GetEntryCount(); nEntry++)
                    {
                        sal_uInt16 nEntryData = (sal_uInt16)reinterpret_cast<sal_uLong>(rFmtLB.GetEntryData(nEntry));
                        if(nEntryData == (sal_uInt16) nCurrent)
                        {
                            bInsert = false;
                            aRemove[nEntry] = nDontRemove;
                            break;
                        }
                    }
                    if(bInsert)
                    {
                        OUString aIdent = xInfo->getNumberingIdentifier( nCurrent );
                        sal_Int32 nPos = rFmtLB.InsertEntry(aIdent);
                        rFmtLB.SetEntryData(nPos, reinterpret_cast<void*>((sal_uLong)nCurrent));
                    }
                }
            }
        }
        for (unsigned short i : aRemove)
        {
            if (i != nDontRemove)
            {
                sal_Int32 nPos = rFmtLB.GetEntryPos( reinterpret_cast<void*>((sal_uLong)i));
                rFmtLB.RemoveEntry( nPos);
            }
        }
    }
};

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
