/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: servuno.hxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-10 13:10:06 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

class String;
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

#define SC_SERVICE_DOCSPRSETT   22
#define SC_SERVICE_DOCCONF      23

#define SC_SERVICE_IMAP_RECT    24
#define SC_SERVICE_IMAP_CIRC    25
#define SC_SERVICE_IMAP_POLY    26

// #100263# Support creation of GraphicObjectResolver and EmbeddedObjectResolver
#define SC_SERVICE_EXPORT_GOR   27
#define SC_SERVICE_IMPORT_GOR   28
#define SC_SERVICE_EXPORT_EOR   29
#define SC_SERVICE_IMPORT_EOR   30

#define SC_SERVICE_VALBIND      31
#define SC_SERVICE_LISTCELLBIND 32
#define SC_SERVICE_LISTSOURCE   33

#define SC_SERVICE_CELLADDRESS  34
#define SC_SERVICE_RANGEADDRESS 35

#define SC_SERVICE_SHEETDOCSET  36

// BM
#define SC_SERVICE_CHDATAPROV   37

#define SC_SERVICE_FORMULAPARS  38
#define SC_SERVICE_OPCODEMAPPER 39

#define SC_SERVICE_COUNT        40
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

