/*************************************************************************
 *
 *  $RCSfile: DIndexPage.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:14:25 $
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

#ifndef _CONNECTIVITY_DBASE_INDEXPAGE_HXX_
#define _CONNECTIVITY_DBASE_INDEXPAGE_HXX_

//#ifndef _REF_HXX
//#include <tools/ref.hxx>
//#endif
#ifndef _VOS_REF_HXX_
#include <vos/ref.hxx>
#endif
#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif
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

            UINT32  nPagePos;       // Position in der Indexdatei

        public:
            ONDXPagePtr(UINT32 nPos = 0):nPagePos(nPos){}
            ONDXPagePtr(const ONDXPagePtr& rRef);
            ONDXPagePtr(ONDXPage* pRefPage);

            ONDXPagePtr& operator=(const ONDXPagePtr& rRef);
            ONDXPagePtr& operator=(ONDXPage* pPageRef);

            UINT32 GetPagePos() const {return nPagePos;}
            BOOL HasPage() const {return nPagePos != 0;}
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

