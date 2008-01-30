/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: HtmlReader.hxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-30 08:46:52 $
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
#ifndef DBAUI_HTMLREADER_HXX
#define DBAUI_HTMLREADER_HXX

#ifndef DBAUI_DATABASEEXPORT_HXX
#include "DExport.hxx"
#endif
#ifndef _PARHTML_HXX //autogen
#include <svtools/parhtml.hxx>
#endif
#ifndef _SVX_SVXENUM_HXX
#include <svx/svxenum.hxx>
#endif
#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTDESCRIPTOR_HPP_
#include <com/sun/star/awt/FontDescriptor.hpp>
#endif


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



