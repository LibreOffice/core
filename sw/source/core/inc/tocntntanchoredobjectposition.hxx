 /*************************************************************************
 *
 *  $RCSfile: tocntntanchoredobjectposition.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-03-08 13:58:51 $
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
#ifndef _TOCNTNTANCHOREDOBJECTPOSITION_HXX
#define _TOCNTNTANCHOREDOBJECTPOSITION_HXX

#ifndef _ANCHOREDOBJECTPOSITION_HXX
#include <anchoredobjectposition.hxx>
#endif

class SwFrm;
class SwTxtFrm;
class SwLayoutFrm;
class SwRect;
class SwFlyFrm;
class SwFlyAtCntFrm;

namespace objectpositioning
{
    class SwToCntntAnchoredObjectPosition : public SwAnchoredObjectPosition
    {
        private:
            // calculated data for object position
            const SwLayoutFrm* mpVertPosOrientFrm;

            // data for calculation of position
            bool          mbAnchorToChar;
            const SwFrm*  mpToCharOrientFrm;
            const SwRect* mpToCharRect;
            SwTwips       mnToCharTopOfLine;

            virtual bool IsAnchoredToChar() const;
            virtual const SwFrm* ToCharOrientFrm() const;
            virtual const SwRect* ToCharRect() const;
            // OD 12.11.2003 #i22341#
            virtual SwTwips ToCharTopOfLine() const;

            // method to cast <SwAnchoredObjectPosition::GetAnchorFrm()> to
            // the needed type
            SwTxtFrm&       GetAnchorTxtFrm() const;

            // method to cast <SwAnchoredObjectPosition::GetFrmOfObj()> to
            // the needed type
            SwFlyAtCntFrm*  GetFlyAtCntFrmOfObj() const;


        // *********************************************************************
            /** determine frame for horizontal position

                OD 04.08.2003
                if the given proposed frame is a content frame, the proposed
                frame is returned.
                otherwise (given proposed frame is a layout frame),
                the lower content frames of the proposed frame are checked
                for the first, that the anchor or a follow of the anchor.
                If none is found, the proposed frame is returned.

                @author OD

                @param _pProposedFrm
                input parameter - proposed frame for horizontal position

                @return constant reference to <SwFrm> object, at which the
                horizontal position is determined.
            */
            const SwFrm& _GetHoriVirtualAnchor( const SwLayoutFrm& _pProposedFrm ) const;

        // *********************************************************************
            /** adjust calculated horizontal position in order to draw object
                aside other objects with same positioning

                @author OD

                @param _rHoriOrientFrm
                input parameter - frame the horizontal position of the object
                is oriented at.

                @param _nProposedRelPosX
                input parameter - proposed relative horizontal position, which
                will be adjusted.

                @param _nRelPosY
                input parameter - relative vertical position

                @param _eHoriOrient
                input parameter - horizontal position of object

                @param _eRelOrient
                inpt parameter - alignment of object

                @param _rLRSpacing
                input parameter - left and right spacing of the object to the text

                @param _rULSpacing
                input parameter - upper and lower spacing of the object to the text

                @param _bEvenPage
                input parameter - boolean indicating, if object is on an even page.

                @return adjusted relative horizontal position in SwTwips
            */
            SwTwips _AdjustHoriRelPosForDrawAside( const SwFrm&  _rHoriOrientFrm,
                                                   const SwTwips _nProposedRelPosX,
                                                   const SwTwips _nRelPosY,
                                                   const SwHoriOrient _eHoriOrient,
                                                   const SwRelationOrient _eRelOrient,
                                                   const SvxLRSpaceItem& _rLRSpacing,
                                                   const SvxULSpaceItem& _rULSpacing,
                                                   const bool _bEvenPage
                                                 ) const;

        // *********************************************************************
            /** detemine, if object has to draw aside given fly frame

                OD 11.08.2003 #110978#
                method used by <_AdjustHoriRelPosForDrawAside(..)>

                @author OD

                @param _pFly
                input parameter - fly frame the draw aside check is done for.

                @param _rObjRect
                input parameter - proposed object rectangle

                @param _pObjContext
                input parameter - context of the object

                @param _nObjIndex
                input parameter - index of the anchor frame of the object

                @param _bEvenPage
                input parameter - boolean indicating, if object is on an even page.

                @param _eHoriOrient
                input parameter - horizontal position of object

                @param _eRelOrient
                inpt parameter - alignment of object

                @return boolean indicating, if object has to be drawn aside
                given fly frame.
            */
            bool _DrawAsideFly( const SwFlyFrm* _pFly,
                                const SwRect&   _rObjRect,
                                const SwFrm*    _pObjContext,
                                const ULONG     _nObjIndex,
                                const bool      _bEvenPage,
                                const SwHoriOrient _eHoriOrient,
                                const SwRelationOrient _eRelOrient
                              ) const;

        // *********************************************************************
            /** determine, if object has to draw aside another object

                OD 04.08.2003
                the different alignments of the objects determines, if one has
                to draw aside another one. Thus, the given alignment are checked
                against each other, which one has to be drawn aside the other one.
                depending on parameter _bLeft check is done for left or right
                positioning.
                method used by <_DrawAsideFly(..)>

                @author OD

                @param _eRelOrient1
                input parameter - alignment 1

                @param _eRelOrient2
                input parameter - alignment 2

                @param _bLeft
                input parameter - boolean indicating, if check is done for left
                or for right positioning.

                @return boolean indicating, if an object with an alignment
                <_eRelOrient1> has to be drawn aside an object with an
                alignment <_eRelOrient2>
            */
            bool _Minor( SwRelationOrient _eRelOrient1,
                         SwRelationOrient _eRelOrient2,
                         bool             _bLeft ) const;

        public:
            SwToCntntAnchoredObjectPosition( SdrObject& _rDrawObj );
            ~SwToCntntAnchoredObjectPosition();

            /** calculate position of object

                @author OD
            */
            virtual void CalcPosition();

            /** frame, at which the vertical position is oriented at

                @author OD
            */
            const SwLayoutFrm& GetVertPosOrientFrm() const;
    };
};

#endif
