/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: atrhndl.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 09:11:35 $
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

#ifndef _ATRHNDL_HXX
#define _ATRHNDL_HXX

#define INITIAL_NUM_ATTR 3
#define NUM_ATTRIBUTE_STACKS 38

#ifndef _TXATBASE_HXX
#include <txatbase.hxx>
#endif
#ifndef _SWFNTCCH_HXX
#include <swfntcch.hxx>
#endif

class SwAttrSet;
class IDocumentSettingAccess;
class ViewShell;
class SfxPoolItem;
extern const BYTE StackPos[];

/*************************************************************************
 *                      class SwAttrHandler
 *
 * Used by Attribute Iterators to organize attributes on stacks to
 * find the valid attribute in each category
 *************************************************************************/

class SwAttrHandler
{
private:

    /*************************************************************************
     *                      class SwAttrStack
     *
     * Container for SwTxtAttr Objects
     *************************************************************************/

    class SwAttrStack
    {
    private:
        SwTxtAttr* pInitialArray[ INITIAL_NUM_ATTR ];
        SwTxtAttr** pArray;
        USHORT nCount; // number of elements on stack
        USHORT nSize;    // number of positions in Array

    public:
        // Ctor, Dtor
        inline SwAttrStack();
        inline ~SwAttrStack() {
            if ( nSize > INITIAL_NUM_ATTR ) delete [] pArray; }

        // reset stack
        inline void Reset() { nCount = 0; };

        // insert on top
        inline void Push( const SwTxtAttr& rAttr ) { Insert( rAttr, nCount ); };
        // insert at specified position, take care for not inserting behind
        // the value returned by Count()
        void Insert( const SwTxtAttr& rAttr, const USHORT nPos );

        // remove specified attribute
        void Remove( const SwTxtAttr& rAttr );

        // get attribute from top if exists, otherwise 0
        const SwTxtAttr* Top() const;

        // number of elements on stack
        inline USHORT Count() const { return nCount; };

        // returns position of rAttr on Stack if found, otherwise USHRT_MAX
        // can be used for Remove of an attribute
        USHORT Pos( const SwTxtAttr& rAttr ) const;
    };

    SwAttrStack aAttrStack[ NUM_ATTRIBUTE_STACKS ]; // stack collection
    const SfxPoolItem* pDefaultArray[ NUM_DEFAULT_VALUES ];
    const IDocumentSettingAccess* mpIDocumentSettingAccess;
    const ViewShell* mpShell;

    // This is the base font for the paragraph. It is stored in order to have
    // a template, if we have to restart the attribute evaluation
    SwFont* pFnt;

    sal_Bool bVertLayout;

    // change font according to pool item
    void FontChg(const SfxPoolItem& rItem, SwFont& rFnt, sal_Bool bPush );

    // push attribute to specified stack, returns true, if attribute has
    // been pushed on top of stack (important for stacks containing different
    // attributes with different priority and redlining)
    sal_Bool Push( const SwTxtAttr& rAttr, const SfxPoolItem& rItem );

    // apply top attribute on stack to font
    void ActivateTop( SwFont& rFnt, USHORT nStackPos );

public:
    // Ctor
    SwAttrHandler();
    ~SwAttrHandler();

    // set default attributes to values in rAttrSet or from cache
    void Init( const SwAttrSet& rAttrSet,
               const IDocumentSettingAccess& rIDocumentSettingAccess,
               const ViewShell* pShell );
    void Init( const SfxPoolItem** pPoolItem, const SwAttrSet* pAttrSet,
               const IDocumentSettingAccess& rIDocumentSettingAccess,
               const ViewShell* pShell, SwFont& rFnt,
               sal_Bool bVertLayout );

    // remove everything from internal stacks, keep default data
    void Reset( );

    // insert specified attribute and change font
    void PushAndChg( const SwTxtAttr& rAttr, SwFont& rFnt );

    // remove specified attribute and reset font
    void PopAndChg( const SwTxtAttr& rAttr, SwFont& rFnt );
    void Pop( const SwTxtAttr& rAttr );

    // apply script dependent attributes
//    void ChangeScript( SwFont& rFnt, const BYTE nScr );

    // returns the default value for stack nStack
    inline const SfxPoolItem& GetDefault( const USHORT nAttribID ) const;
    // do not call these if you only used the small init function
    inline void ResetFont( SwFont& rFnt ) const;
    inline const SwFont* GetFont() const;

    void GetDefaultAscentAndHeight(ViewShell* pShell,
                                   OutputDevice& rOut,
                                   USHORT& nAscent,
                                   USHORT& nHeight) const;
};

inline const SfxPoolItem& SwAttrHandler::GetDefault( const USHORT nAttribID ) const
{
    ASSERT( nAttribID < RES_TXTATR_END,
            "this attrib does not ex."
            );
    ASSERT( pDefaultArray[ StackPos[ nAttribID ] ], "array not initialized" );
    return *pDefaultArray[ StackPos[ nAttribID ] ];
}

inline void SwAttrHandler::ResetFont( SwFont& rFnt ) const
{
    ASSERT( pFnt, "ResetFont without a font" );
    if ( pFnt )
        rFnt = *pFnt;
};

inline const SwFont* SwAttrHandler::GetFont() const
{
    return pFnt;
};

#endif
