/*************************************************************************
 *
 *  $RCSfile: accframe.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: mib $ $Date: 2002-02-04 14:07:14 $
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
#ifndef _SV_GEN_HXX
#include <vcl/gen.hxx>
#endif

class SwFrm;
class Window;

class SwAccessibleFrame
{
    Rectangle aVisArea;
    const SwFrm *pFrm;

    static sal_Bool IsAccessible( const SwFrm *pFrm );
    static sal_Int32 GetLowerCount( const Rectangle& rVisArea,
                                    const SwFrm *pFrm );
    static const SwFrm *GetDescendant( const Rectangle& rVisArea,
                                       const SwFrm *pFrm,
                                       sal_Int32& rPos );
    static sal_Bool GetDescendantIndex( const Rectangle& rVisArea,
                                        const SwFrm *pFrm,
                                        const SwFrm *pChild,
                                        sal_Int32& rPos );
    static const SwFrm *GetDescendantAt( const Rectangle& rVisArea,
                                         const SwFrm *pFrm,
                                           const Point& rPos );
    static void SetVisArea( SwAccessibleFrame *pAcc,
                            const SwFrm *pFrm,
                            const Rectangle& rOldVisArea );
protected:

    // A child has been added while setting the vis area
    virtual void LowerAdded( const SwFrm *pFrm );

    // A child has been removed while setting the vis area
    virtual void LowerRemoved( const SwFrm *pFrm );

    // A child has been moved while setting the vis area
    virtual void LowerMoved( const SwFrm *pFrm );

    Rectangle GetBounds( const SwFrm *pFrm=0 );

    Window *GetWindow();

    void ClearFrm() { pFrm = 0; }

public:

    SwAccessibleFrame( const Rectangle& rVisArea, const SwFrm *pFrm );
    virtual ~SwAccessibleFrame();

    // Return the SwFrm this context is attached to.
    const SwFrm *GetFrm() const { return pFrm; };

    // Return the upper that has a context attached. This might be
    // another one than the immediate upper.
    const SwFrm *GetUpper() const;

    // Return the lower count or the nth lower, there the lowers have a
    // not be same one as the SwFrm's lowers
    inline sal_Int32 GetLowerCount() const;
    inline const SwFrm *GetLower( sal_Int32 nPos ) const;
    inline sal_Int32 GetLowerIndex( const SwFrm *pFrm ) const;
    inline const SwFrm *GetDescendantAt( const Point& rPos ) const;

    void SetVisArea( const Rectangle& rNewVisArea );
    const Rectangle& GetVisArea() const { return aVisArea; }
};

inline sal_Int32 SwAccessibleFrame::GetLowerCount() const
{
    return GetLowerCount( aVisArea, pFrm );
}
inline const SwFrm *SwAccessibleFrame::GetLower( sal_Int32 nPos ) const
{
    return GetDescendant( aVisArea, pFrm, nPos );
}

inline sal_Int32 SwAccessibleFrame::GetLowerIndex( const SwFrm *pChild ) const
{
    sal_Int32 nPos = 0;
    return GetDescendantIndex( aVisArea, pFrm, pChild, nPos ) ? nPos : -1L;
}

inline const SwFrm *SwAccessibleFrame::GetDescendantAt( const Point& rPos ) const
{
    return GetDescendantAt( aVisArea, pFrm, rPos );
}

#endif

