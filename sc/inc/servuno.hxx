/*************************************************************************
 *
 *  $RCSfile: servuno.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: sab $ $Date: 2001-03-29 05:33:40 $
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
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef SC_SERVUNO_HXX
#define SC_SERVUNO_HXX

#ifndef _COM_SUN_STAR_UNO_XINTERFACE_HPP_
#include <com/sun/star/uno/XInterface.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
//#ifndef _USR_USTRING_HXX
//#include <usr/ustring.hxx>
//#endif

class ScDocShell;


//! AutoFormat wirklich hier oder besser global??????

#define SC_SERVICE_SHEET        0
#define SC_SERVICE_URLFIELD     1
#define SC_SERVICE_PAGEFIELD    2
#define SC_SERVICE_PAGESFIELD   3
#define SC_SERVICE_DATEFIELD    4
#define SC_SERVICE_TIMEFIELD    5
#define SC_SERVICE_TITLEFIELD   6
#define SC_SERVICE_FILEFIELD    7
#define SC_SERVICE_SHEETFIELD   8
#define SC_SERVICE_CELLSTYLE    9
#define SC_SERVICE_PAGESTYLE    10
#define SC_SERVICE_AUTOFORMAT   11
#define SC_SERVICE_CELLRANGES   12

//  drawing layer tables
#define SC_SERVICE_GRADTAB      13
#define SC_SERVICE_HATCHTAB     14
#define SC_SERVICE_BITMAPTAB    15
#define SC_SERVICE_TRGRADTAB    16
#define SC_SERVICE_MARKERTAB    17
#define SC_SERVICE_DASHTAB      18
#define SC_SERVICE_NUMRULES     19

#define SC_SERVICE_DOCDEFLTS    20
#define SC_SERVICE_DRAWDEFLTS   21

#define SC_SERVICE_DOCCONF      22

#define SC_SERVICE_COUNT        23
#define SC_SERVICE_INVALID      USHRT_MAX


class ScServiceProvider
{
public:
                            // pDocShell wird nicht fuer alle Services benoetigt
    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                            MakeInstance( sal_uInt16 nType, ScDocShell* pDocShell );
    static ::com::sun::star::uno::Sequence<rtl::OUString> GetAllServiceNames();
    static String           GetProviderName(sal_uInt16 nObjectType);
    static sal_uInt16       GetProviderType(const String& rServiceName);
};



#endif

