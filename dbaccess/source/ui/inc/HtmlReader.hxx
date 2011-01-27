/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef DBAUI_HTMLREADER_HXX
#define DBAUI_HTMLREADER_HXX

#include "DExport.hxx"
#include <svtools/parhtml.hxx>
#include <editeng/svxenum.hxx>
#include <tools/stream.hxx>
#include <com/sun/star/awt/FontDescriptor.hpp>


namespace dbaui
{
    //===============================================================================================
    // OHTMLReader
    //===============================================================================================
    class OHTMLReader : public HTMLParser, public ODatabaseExport
    {
        String              m_sCurrent;
        sal_Int32           m_nTableCount;
        sal_Int16           m_nWidth;
        sal_Int16           m_nColumnWidth;     // max. Spaltenbreite
        sal_Bool            m_bMetaOptions;     // true when we scaned the meta information
        sal_Bool            m_bSDNum;

    protected:
        virtual void        NextToken( int nToken ); // Basisklasse
        virtual sal_Bool    CreateTable(int nToken);
        virtual TypeSelectionPageFactory
                             getTypeSelectionPageFactory();

        void                TableDataOn(SvxCellHorJustify& eVal,int nToken);
        void                TableFontOn(::com::sun::star::awt::FontDescriptor& _rFont,sal_Int32 &_rTextColor);
        sal_Int16           GetWidthPixel( const HTMLOption* pOption );
        void                setTextEncoding();
        void                fetchOptions();
        virtual ~OHTMLReader();
    public:
        OHTMLReader(SvStream& rIn,
                    const SharedConnection& _rxConnection,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& _rxNumberF,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rM,
                    const TColumnVector* rList = 0,
                    const OTypeInfoMap* _pInfoMap = 0);
        // wird f"ur auto. Typ-Erkennung gebraucht
        OHTMLReader(SvStream& rIn,
                    sal_Int32 nRows,
                    const TPositions &_rColumnPositions,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& _rxNumberF,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rM,
                    const TColumnVector* rList,
                    const OTypeInfoMap* _pInfoMap,
                    sal_Bool _bAutoIncrementEnabled);

        virtual     SvParserState CallParser();// Basisklasse
        virtual     void          release();
        // birgt nur korrekte Daten, wenn der 1. CTOR benutzt wurde
    };

    SV_DECL_IMPL_REF( OHTMLReader );
}
#endif // DBAUI_HTMLREADER_HXX



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
