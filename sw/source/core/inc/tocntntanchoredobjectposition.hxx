 /*************************************************************************
 *
 *  $RCSfile: tocntntanchoredobjectposition.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hjs $ $Date: 2004-06-28 13:36:58 $
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
            // --> OD 2004-06-17 #i26791#
            // determine offset to frame anchor position according to the
            // positioning alignments
            Point maOffsetToFrmAnchorPos;

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

            /** determined offset to frame anchor position

                --> OD 2004-06-17 #i26791#

                @author OD
            */
            Point GetOffsetToFrmAnchorPos() const;
    };
};

#endif
