/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: accframe.hxx,v $
 *
 *  $Revision: 1.18 $
 *
 *  last change: $Author: hr $ $Date: 2007-08-03 13:33:54 $
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
#ifndef _ACCFRAME_HXX
#define _ACCFRAME_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#include <list>
#ifndef _ACCFRMOBJ_HXX
#include <accfrmobj.hxx>
#endif

// Any method of this class must be called with an acquired solar mutex!

class SwAccessibleMap;

class SwAccessibleFrame
{
    SwRect maVisArea;
    const SwFrm* mpFrm;
    const sal_Bool mbIsInPagePreview;

protected:
    // --> OD 2007-06-29 #i77106#
    // method needs to be called by new class <SwAccessibleTableColHeaders>
    static sal_Int32 GetChildCount( const SwRect& rVisArea,
                                    const SwFrm *pFrm,
                                    sal_Bool bInPagePreview );
private:
    static SwFrmOrObj GetChild( const SwRect& rVisArea,
                                  const SwFrm *pFrm,
                                  sal_Int32& rPos,
                                sal_Bool bInPagePreview);
    static sal_Bool GetChildIndex( const SwRect& rVisArea,
                                    const SwFrm *pFrm,
                                    const SwFrmOrObj& rChild,
                                    sal_Int32& rPos,
                                    sal_Bool bInPagePreview );
    static SwFrmOrObj GetChildAtPixel( const SwRect& rVisArea,
                                    const SwFrm *pFrm,
                                    const Point& rPos,
                                    sal_Bool bInPagePreview,
                                    const SwAccessibleMap *pMap );
    static void GetChildren( const SwRect& rVisArea, const SwFrm *pFrm,
                             ::std::list< SwFrmOrObj >& rChildren,
                             sal_Bool bInPagePreview );

protected:

    sal_Bool IsEditable( ViewShell *pVSh ) const;

    sal_Bool IsOpaque( ViewShell *pVSh ) const;

    inline sal_Bool IsShowing( const SwRect& rFrm ) const;
    inline sal_Bool IsShowing( const SwFrmOrObj& rFrmOrObj ) const;
    inline sal_Bool IsShowing() const;

    inline sal_Bool IsInPagePreview() const
    {
        return mbIsInPagePreview;
    }

    inline void ClearFrm()
    {
        mpFrm = 0;
    }

    SwAccessibleFrame( const SwRect& rVisArea,
                       const SwFrm *pFrm,
                       sal_Bool bIsPagePreview );
    virtual ~SwAccessibleFrame();

public:
    // Return the SwFrm this context is attached to.
    const SwFrm *GetFrm() const { return mpFrm; };


    static const SwFrm *GetParent( const SwFrmOrObj& rFrmOrObj,
                                   sal_Bool bInPagePreview );

protected:

    // Return the bounding box of the frame clipped to the vis area. If
    // no frame is specified, use this' frame.
    SwRect GetBounds( const SwFrm *pFrm=0 );

    // Return the upper that has a context attached. This might be
    // another one than the immediate upper.
    inline const SwFrm *GetParent() const;

    // Return the lower count or the nth lower, there the lowers have a
    // not be same one as the SwFrm's lowers
    inline sal_Int32 GetChildCount() const;
    inline SwFrmOrObj GetChild( sal_Int32 nPos ) const;
    inline sal_Int32 GetChildIndex( const SwFrmOrObj& rChild ) const;
    inline SwFrmOrObj GetChildAtPixel( const Point& rPos,
                                    const SwAccessibleMap *pMap ) const;
    inline void GetChildren( ::std::list< SwFrmOrObj >& rChildren ) const;

    inline void SetVisArea( const SwRect& rNewVisArea )
    {
        maVisArea = rNewVisArea;
    }

    inline const SwRect& GetVisArea() const
    {
        return maVisArea;
    }


    String GetFormattedPageNumber() const;
};

inline sal_Bool SwAccessibleFrame::IsShowing( const SwRect& rFrm ) const
{
    return rFrm.IsOver( maVisArea );
}

inline sal_Bool SwAccessibleFrame::IsShowing( const SwFrmOrObj& rFrmOrObj ) const
{
    return IsShowing( rFrmOrObj.GetBox() );
}

inline sal_Bool SwAccessibleFrame::IsShowing() const
{
    SwFrmOrObj aFrmOrObj( GetFrm() );
    return IsShowing( aFrmOrObj );
}

inline const SwFrm *SwAccessibleFrame::GetParent() const
{
    SwFrmOrObj aFrmOrObj( GetFrm() );
    return GetParent( aFrmOrObj, IsInPagePreview()  );
}

inline sal_Int32 SwAccessibleFrame::GetChildCount() const
{
    return GetChildCount( maVisArea, mpFrm, IsInPagePreview() );
}

inline SwFrmOrObj SwAccessibleFrame::GetChild( sal_Int32 nPos ) const
{
    return GetChild( maVisArea, mpFrm, nPos, IsInPagePreview() );
}

inline sal_Int32 SwAccessibleFrame::GetChildIndex( const SwFrmOrObj& rChild ) const
{
    sal_Int32 nPos = 0;
    return GetChildIndex( maVisArea, mpFrm, rChild, nPos, IsInPagePreview() ) ? nPos : -1L;
}

inline SwFrmOrObj SwAccessibleFrame::GetChildAtPixel( const Point& rPos,
                                    const SwAccessibleMap *pMap ) const
{
    return GetChildAtPixel( maVisArea, mpFrm, rPos, IsInPagePreview(), pMap );
}

inline void SwAccessibleFrame::GetChildren( ::std::list< SwFrmOrObj >& rChildren ) const
{
    GetChildren( maVisArea, mpFrm, rChildren, IsInPagePreview() );
}
#endif

