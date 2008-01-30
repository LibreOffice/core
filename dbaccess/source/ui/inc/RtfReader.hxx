/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: RtfReader.hxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-30 08:47:22 $
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


