/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef DBAUI_HTMLREADER_HXX
#define DBAUI_HTMLREADER_HXX

#ifndef DBAUI_DATABASEEXPORT_HXX
#include "DExport.hxx"
#endif
#ifndef _PARHTML_HXX //autogen
#include <svtools/parhtml.hxx>
#endif
#ifndef _SVX_SVXENUM_HXX
#include <editeng/svxenum.hxx>
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



