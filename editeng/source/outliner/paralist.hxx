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



#ifndef _PARALIST_HXX
#define _PARALIST_HXX

class Paragraph;

#include <tools/list.hxx>
#include <tools/link.hxx>

class ParagraphList : private List
{
private:
    Link            aVisibleStateChangedHdl;

public:
    void            Clear( sal_Bool bDestroyParagraphs );

    sal_uLong           GetParagraphCount() const           { return List::Count(); }
    Paragraph*      GetParagraph( sal_uLong nPos ) const    { return (Paragraph*)List::GetObject( nPos ); }

    sal_uLong           GetAbsPos( Paragraph* pParent ) const { return List::GetPos( pParent ); }
    sal_uLong           GetVisPos( Paragraph* pParagraph );

    void            Insert( Paragraph* pPara, sal_uLong nAbsPos = LIST_APPEND ) { List::Insert( pPara, nAbsPos ); }
    void            Remove( sal_uLong nPara ) { List::Remove( nPara ); }
    void            MoveParagraphs( sal_uLong nStart, sal_uLong nDest, sal_uLong nCount );

    Paragraph*      NextVisible( Paragraph* ) const;
    Paragraph*      PrevVisible( Paragraph* ) const;
    Paragraph*      LastVisible() const;

    Paragraph*      GetParent( Paragraph* pParagraph /*, sal_uInt16& rRelPos */ ) const;
    sal_Bool            HasChilds( Paragraph* pParagraph ) const;
    sal_Bool            HasHiddenChilds( Paragraph* pParagraph ) const;
    sal_Bool            HasVisibleChilds( Paragraph* pParagraph ) const;
    sal_uLong           GetChildCount( Paragraph* pParagraph ) const;

    void            Expand( Paragraph* pParent );
    void            Collapse( Paragraph* pParent );

    void            SetVisibleStateChangedHdl( const Link& rLink ) { aVisibleStateChangedHdl = rLink; }
    Link            GetVisibleStateChangedHdl() const { return aVisibleStateChangedHdl; }
};

#endif
