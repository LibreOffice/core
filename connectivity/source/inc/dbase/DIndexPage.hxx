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

