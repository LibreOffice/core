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


#ifndef DBAUI_RTFREADER_HXX
#define DBAUI_RTFREADER_HXX

#ifndef _VECTOR_
#include <vector>
#endif
#ifndef _PARRTF_HXX //autogen
#include <svtools/parrtf.hxx>
#endif
#ifndef DBAUI_DATABASEEXPORT_HXX
#include "DExport.hxx"
#endif
#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif

namespace dbaui
{
    class ORTFReader : public SvRTFParser , public ODatabaseExport
    {
        ::std::vector<sal_Int32>    m_vecColor;

        //  void insertValueIntoColumn();
    protected:
        virtual sal_Bool        CreateTable(int nToken);
        virtual void            NextToken( int nToken ); // Basisklasse
        virtual TypeSelectionPageFactory
                                getTypeSelectionPageFactory();

        ~ORTFReader();
    public:
        ORTFReader( SvStream& rIn,
                    const SharedConnection& _rxConnection,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& _rxNumberF,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rM,
                    const TColumnVector* rList = 0,
                    const OTypeInfoMap* _pInfoMap = 0);
        // wird f"ur auto. Typ-Erkennung gebraucht
        ORTFReader( SvStream& rIn,
                    sal_Int32 nRows,
                    const TPositions &_rColumnPositions,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& _rxNumberF,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rM,
                    const TColumnVector* rList,
                    const OTypeInfoMap* _pInfoMap,
                    sal_Bool _bAutoIncrementEnabled);

        virtual SvParserState   CallParser();// Basisklasse
        virtual void            release();
        // birgt nur korrekte Daten, wenn der 2. CTOR benutzt wurde
        // ansonsten wird die SbaColumnList ohne "Anderung zur"uckgegeben
    };

    SV_DECL_IMPL_REF( ORTFReader );
}
#endif // DBAUI_RTFREADER_HXX


