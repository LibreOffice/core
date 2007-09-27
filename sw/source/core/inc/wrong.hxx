/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: wrong.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 09:00:27 $
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

#ifndef _WRONG_HXX
#define _WRONG_HXX

#ifndef _COM_SUN_STAR_SMARTTAGS_XSMARTTAGPROPERTIES_HPP_
#include <com/sun/star/container/XStringKeyMap.hpp>
#endif

#include <vector>

#include <tools/string.hxx>

class SwWrongList;

// ST2
class SwWrongArea
{
public:
    rtl::OUString maType;
    com::sun::star::uno::Reference< com::sun::star::container::XStringKeyMap > mxPropertyBag;
    xub_StrLen mnPos;
    xub_StrLen mnLen;
    SwWrongList* mpSubList;

    SwWrongArea() : mnPos(0), mnLen(0), mpSubList(NULL) {}
    SwWrongArea( const rtl::OUString& rType,
                 com::sun::star::uno::Reference< com::sun::star::container::XStringKeyMap > xPropertyBag,
                 xub_StrLen nPos,
                 xub_StrLen nLen,
                 SwWrongList* pSubList )
        : maType(rType), mxPropertyBag(xPropertyBag), mnPos(nPos), mnLen(nLen), mpSubList(pSubList) {}
};

class SwWrongList
{
    std::vector<SwWrongArea> maList;

    xub_StrLen nBeginInvalid;   // Start des ungueltigen Bereichs
    xub_StrLen nEndInvalid;     // Ende des ungueltigen Bereichs
    void ShiftLeft( xub_StrLen &rPos, xub_StrLen nStart, xub_StrLen nEnd )
    { if( rPos > nStart ) rPos = rPos > nEnd ? rPos - nEnd + nStart : nStart; }
    void ShiftRight( xub_StrLen &rPos, xub_StrLen nStart, xub_StrLen nEnd )
    { if( rPos >= nStart ) rPos += nStart - nEnd; }
    void _Invalidate( xub_StrLen nBegin, xub_StrLen nEnd );

    void Insert(USHORT nWhere, std::vector<SwWrongArea>::iterator startPos, std::vector<SwWrongArea>::iterator endPos);
    void Remove( USHORT nIdx, USHORT nLen );

    // forbidden and not implemented
    SwWrongList( const SwWrongList& rCpy );
    SwWrongList& operator= (const SwWrongList &);

public:

    SwWrongList();
    ~SwWrongList()
    {
        for ( unsigned int i=0; i< maList.size(); i++)
        {
            if (maList[i].mpSubList)
                delete maList[i].mpSubList;
            maList[i].mpSubList = NULL;
        }
        //maList.resize(0);
    }

    inline xub_StrLen GetBeginInv() const { return nBeginInvalid; }
    inline xub_StrLen GetEndInv() const { return nEndInvalid; }
    inline BOOL InsideInvalid( xub_StrLen nChk ) const
        { return nChk >= nBeginInvalid && nChk <= nEndInvalid; }
    inline void SetInvalid( xub_StrLen nBegin, xub_StrLen nEnd )
        { nBeginInvalid = nBegin; nEndInvalid = nEnd; }
    inline void Validate(){ nBeginInvalid = STRING_LEN; }
    inline void Invalidate( xub_StrLen nBegin, xub_StrLen nEnd )
        { if( STRING_LEN == GetBeginInv() ) SetInvalid( nBegin, nEnd );
          else _Invalidate( nBegin, nEnd ); }
    BOOL InvalidateWrong();
    BOOL Fresh( xub_StrLen &rStart, xub_StrLen &rEnd, xub_StrLen nPos,
            xub_StrLen nLen, USHORT nIndex, xub_StrLen nCursorPos );
    USHORT GetWrongPos( xub_StrLen nValue ) const;

    sal_Bool Check( xub_StrLen &rChk, xub_StrLen &rLn ) const;
    sal_Bool InWrongWord( xub_StrLen &rChk, xub_StrLen &rLn ) const;
    xub_StrLen NextWrong( xub_StrLen nChk ) const;

    void Move( xub_StrLen nPos, long nDiff );

    // Divide the list into two part, the wrong words until nSplitPos will be
    // removed and transferred to a new SwWrongList.
    SwWrongList* SplitList( xub_StrLen nSplitPos );
    // Join the next SwWrongList, nInsertPos is my own text length, where
    // the other wrong list has to be inserted.
    void JoinList( SwWrongList* pNext, xub_StrLen nInsertPos );

    inline xub_StrLen Len( USHORT nIdx ) const
    {
        return nIdx < maList.size() ? maList[nIdx].mnLen : 0;
    }

    inline xub_StrLen Pos( USHORT nIdx ) const
    {
        return nIdx < maList.size() ? maList[nIdx].mnPos : 0;
    }

    inline USHORT Count() const { return (USHORT)maList.size(); }

    inline void Insert( const rtl::OUString& rType,
                        com::sun::star::uno::Reference< com::sun::star::container::XStringKeyMap > xPropertyBag,
                        xub_StrLen nNewPos, xub_StrLen nNewLen, USHORT nWhere )
    {
        std::vector<SwWrongArea>::iterator i = maList.begin();
        if ( nWhere >= maList.size() )
            i = maList.end(); // robust
        else
            i += nWhere;
        maList.insert(i, SwWrongArea( rType, xPropertyBag, nNewPos, nNewLen, 0 ) );
    }

    void Insert( const rtl::OUString& rType,
                 com::sun::star::uno::Reference< com::sun::star::container::XStringKeyMap > xPropertyBag,
                 xub_StrLen nNewPos, xub_StrLen nNewLen );

    inline SwWrongList* SubList( USHORT nIdx ) const
    {
        return nIdx < maList.size() ? maList[nIdx].mpSubList : 0;
    }

    inline void InsertSubList( xub_StrLen nNewPos, xub_StrLen nNewLen, USHORT nWhere, SwWrongList* pSubList )
    {
        std::vector<SwWrongArea>::iterator i = maList.begin();
        if ( nWhere >= maList.size() )
            i = maList.end(); // robust
        else
            i += nWhere;
        maList.insert(i, SwWrongArea( rtl::OUString(), 0, nNewPos, nNewLen, pSubList ) );
    }

    inline const SwWrongArea* GetElement( USHORT nIdx ) const
    {
        return nIdx < maList.size() ? &maList[nIdx] : 0;
    }
};

#endif
