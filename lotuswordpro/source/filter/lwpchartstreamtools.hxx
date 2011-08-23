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
/*****************************************************************************
* Change History
* <<Date>> <<Name of editor>> <<Description>>
2005/2		draft code for chart stream helpers
****************************************************************************/

/**
 * @file
 *  For LWP filter architecture prototype
*/

#ifndef LWPCHARTSTREAMTOOLS_HXX_
#define LWPCHARTSTREAMTOOLS_HXX_

#include <tools/stream.hxx>
#include "bento.hxx"
#include <tools/gen.hxx>
#include <so3/ipobj.hxx>

#include "lwpobjid.hxx"
#include "lwpobjfactory.hxx"

//For sax parser
#ifndef _COM_SUN_STAR_XML_SAX_INPUTSOURCE_HPP_
#include <com/sun/star/xml/sax/InputSource.hpp>
#endif

#ifndef _COM_SUN_STAR_XML_SAX_XDTDHANDLER_HPP_
#include <com/sun/star/xml/sax/XDTDHandler.hpp>
#endif

#ifndef _COM_SUN_STAR_XML_SAX_XPARSER_HPP_
#include <com/sun/star/xml/sax/XParser.hpp>
#endif

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

//using namespace ::com::sun::star;
using namespace ::rtl;
using namespace comphelper;

#include "xfilter/xfchartxmlhandler.hxx"
#include "xfilter/xfrect.hxx"
class IXFContent;

/**
* @brief
* tools class for chart stream process
*/

class LwpChartStreamTools
{
public:
    static SvStream* GetChartStream(LwpSvStream* pDocStream, const char* pChartName);

    static void OutputChart(XFRect aChartRect, IXFContent* pXChartStrm, IXFStream* pOutputStream);

    //Added by  for XFConvert refactor, 03/31/2005
    static void OutputChart(XFRect aChartRect, IXFContent* pXChartStrm, XFContentContainer* pCont, OUString strStyleName);
    //End of Add

    static INT32 GetStreamLen(SvStream* pStream);

    static void MakeChartStorage(	SvStream* pStream,
        Rectangle aChartRect,
        SvInPlaceObjectRef& xIPObj,
        SvStorageRef& xStorage);

    static void ChartToXML(SvStorageRef xStorage, SvInPlaceObjectRef xIPObj);

    static SvStorageStreamRef GetStream(SvStorageRef xStorage, String aStrmName);

    static void DumpStream(SvStorageStreamRef rStream, char* aName);

    static const char* GetChartName(LwpObjectID* pID);

public:
    static void ParseChart(	LwpSvStream* pDocStream,
                            LwpObjectID* pID,
                            Rectangle aRectIn100thMM,
                            XFRect aRectInCM,
                            IXFStream* pOutputStream);
    //Added by  for XFConvert refactor, 03/31/2005
    static void ParseChart(	LwpSvStream* pDocStream,
                            LwpObjectID* pID,
                            Rectangle aRectIn100thMM,
                            XFRect aRectInCM,
                            XFContentContainer* pCont,
                            OUString strStyleName);
    //End of Add
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
