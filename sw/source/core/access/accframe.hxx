/*************************************************************************
 *
 *  $RCSfile: accframe.hxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: mib $ $Date: 2002-05-15 13:17:31 $
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
#ifndef _ACCFRAME_HXX
#define _ACCFRAME_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif
#include <list>
#ifndef _FRAME_HXX
#include <frame.hxx>
#endif
#ifndef _ACCFRMOBJ_HXX
#include <accfrmobj.hxx>
#endif

// Any method of this class must be called with an acquired solar mutex!

class SwAccessibleFrame
{
    SwRect aVisArea;
    const SwFrm *pFrm;

    static sal_Int32 GetChildCount( const SwRect& rVisArea,
                                    const SwFrm *pFrm );
    static SwFrmOrObj GetChild( const SwRect& rVisArea,
                                  const SwFrm *pFrm,
                                  sal_Int32& rPos );
    static sal_Bool GetChildIndex( const SwRect& rVisArea,
                                    const SwFrm *pFrm,
                                    const SwFrmOrObj& rChild,
                                    sal_Int32& rPos );
    static SwFrmOrObj GetChildAt( const SwRect& rVisArea,
                                    const SwFrm *pFrm,
                                    const Point& rPos );
    static void GetChildren( const SwRect& rVisArea, const SwFrm *pFrm,
                             ::std::list< SwFrmOrObj >& rChildren );

    static void MergeLowerBounds( SwRect& rBounds,
                                  const SwRect& rVisArea,
                                  const SwFrm *pFrm );
protected:

    sal_Bool IsEditable( ViewShell *pVSh ) const;

    sal_Bool IsOpaque( ViewShell *pVSh ) const;

    inline sal_Bool IsShowing( const SwRect& rFrm ) const;
    inline sal_Bool IsShowing( const SwFrmOrObj& rFrmOrObj ) const;
    inline sal_Bool IsShowing() const;

    void ClearFrm() { pFrm = 0; }

    SwAccessibleFrame( const SwRect& rVisArea,
                       const SwFrm *pFrm );
    virtual ~SwAccessibleFrame();

public:
    // Return the SwFrm this context is attached to.
    const SwFrm *GetFrm() const { return pFrm; };


    static const SwFrm *GetParent( const SwFrmOrObj& rFrmOrObj );

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
    inline SwFrmOrObj GetChildAt( const Point& rPos ) const;
    inline void GetChildren( ::std::list< SwFrmOrObj >& rChildren ) const;

    inline void SetVisArea( const SwRect& rNewVisArea );
    const SwRect& GetVisArea() const { return aVisArea; }


    String GetFormattedPageNumber() const;
};

inline sal_Bool SwAccessibleFrame::IsShowing( const SwRect& rFrm ) const
{
    return rFrm.IsOver( aVisArea );
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
    return GetParent( aFrmOrObj );
}

inline sal_Int32 SwAccessibleFrame::GetChildCount() const
{
    return GetChildCount( aVisArea, pFrm );
}

inline SwFrmOrObj SwAccessibleFrame::GetChild( sal_Int32 nPos ) const
{
    return GetChild( aVisArea, pFrm, nPos );
}

inline sal_Int32 SwAccessibleFrame::GetChildIndex( const SwFrmOrObj& rChild ) const
{
    sal_Int32 nPos = 0;
    return GetChildIndex( aVisArea, pFrm, rChild, nPos ) ? nPos : -1L;
}

inline SwFrmOrObj SwAccessibleFrame::GetChildAt( const Point& rPos ) const
{
    return GetChildAt( aVisArea, pFrm, rPos );
}

inline void SwAccessibleFrame::GetChildren(
        ::std::list< SwFrmOrObj >& rChildren ) const
{
    GetChildren( aVisArea, pFrm, rChildren );
}

inline void SwAccessibleFrame::SetVisArea( const SwRect& rNewVisArea )
{
    aVisArea = rNewVisArea;
}
#endif

