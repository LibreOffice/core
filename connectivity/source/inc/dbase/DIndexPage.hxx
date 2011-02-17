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

#ifndef _CONNECTIVITY_DBASE_INDEXPAGE_HXX_
#define _CONNECTIVITY_DBASE_INDEXPAGE_HXX_

//#ifndef _REF_HXX
//#include <tools/ref.hxx>
//#endif
#include <vos/ref.hxx>
#include <tools/stream.hxx>
#ifndef _VECTOR_
#include <vector>
#endif
//#ifndef _CONNECTIVITY_FILE_FCODE_HXX_
//#include "file/fcode.hxx"
//#endif
//#ifndef _CONNECTIVITY_DBASE_INDEXNODE_HXX_
//#include "dbase/dindexnode.hxx"
//#endif

#if 0
namespace connectivity
{
    namespace dbase
    {
        //==================================================================
        // Index Seitenverweis
        //==================================================================
        //  SV_DECL_REF(ONDXPage); // Basisklasse da weitere Informationen gehalten werden muessen

        class ONDXPage;
        typedef vos::ORef<ONDXPage>         ONDXPagePtr_BASE;

        class ONDXPagePtr : public ONDXPagePtr_BASE //ONDXPageRef
        {
            friend  SvStream& operator << (SvStream &rStream, const ONDXPagePtr&);
            friend  SvStream& operator >> (SvStream &rStream, ONDXPagePtr&);

            sal_uInt32  nPagePos;       // Position in der Indexdatei

        public:
            ONDXPagePtr(sal_uInt32 nPos = 0):nPagePos(nPos){}
            ONDXPagePtr(const ONDXPagePtr& rRef);
            ONDXPagePtr(ONDXPage* pRefPage);

            ONDXPagePtr& operator=(const ONDXPagePtr& rRef);
            ONDXPagePtr& operator=(ONDXPage* pPageRef);

            sal_uInt32 GetPagePos() const {return nPagePos;}
            sal_Bool HasPage() const {return nPagePos != 0;}
            sal_Bool Is() const { return isValid(); }
            void Clear()
            {
                unbind();
            }
        };

        SvStream& operator << (SvStream &rStream, const ONDXPagePtr&);
        SvStream& operator >> (SvStream &rStream, ONDXPagePtr&);
    }
}
#endif
#endif // _CONNECTIVITY_DBASE_INDEXPAGE_HXX_

