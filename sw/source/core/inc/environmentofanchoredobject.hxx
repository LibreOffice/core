 /*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: environmentofanchoredobject.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 08:55:55 $
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
#ifndef _ENVIRONMENTOFANCHOREDOBJECT_HXX
#define _ENVIRONMENTOFANCHOREDOBJECT_HXX

class SwFrm;
class SwLayoutFrm;

namespace objectpositioning
{
    class SwEnvironmentOfAnchoredObject
    {
        private:
            const bool mbFollowTextFlow;

        public:
            /** construtor

                OD 05.11.2003

                @author OD

                @param _bFollowTextFlow
                input parameter - indicates, if the anchored object, for which
                this environment is instantiated, follow the text flow or not
            */
            SwEnvironmentOfAnchoredObject( const bool _bFollowTextFlow );

            /** destructor

                OD 05.11.2003

                @author OD
            */
            ~SwEnvironmentOfAnchoredObject();

            /** determine environment layout frame for possible horizontal object
                positions respectively for alignment to 'page areas'

                OD 05.11.2003
                this is, if object has to follow the text flow:
                - cell frame, if anchored inside a cell
                - fly frame, if anchored inside a fly frame
                otherwise it's the page frame

                this is, if object hasn't to follow the text flow:
                - page frame.
                OD 2005-01-20 #118546# - no exception any more. Thus remove
                parameter <_bForPageAlignment>

                @author OD

                @param _rHoriOrientFrm
                input parameter - frame, at which the horizontal position is
                oriented at (typically it's the anchor frame).
                starting point for the search of the layout frame.

                @return reference to the layout frame, which determines the
                the horizontal environment the object has to be positioned in.
            */
            const SwLayoutFrm& GetHoriEnvironmentLayoutFrm( const SwFrm& _rHoriOrientFrm ) const;

            /** determine environment layout frame for possible vertical object
                positions respectively for alignments to 'page areas'

                OD 05.11.2003
                this is, if object has to follow the text flow:
                - cell frame, if anchored inside a cell
                - fly frame, if anchored inside a fly frame
                - header/footer frame, if anchored inside page header/footer
                - footnote frame, if anchored inside footnote
                otherwise it's the document body frame

                this is, if object hasn't to follow the text flow:
                - page frame.
                OD 2005-01-20 #118546# - no exception any more. Thus remove
                parameter <_bForPageAlignment>

                @author OD

                @param _rVertOrientFrm
                input parameter - frame, at which the vertical position is
                oriented at (typically it's the anchor frame).
                starting point for the search of the layout frame.

                @return reference to the layout frame, which determines the
                the vertical environment the object has to be positioned in.
            */
            const SwLayoutFrm& GetVertEnvironmentLayoutFrm( const SwFrm& _rVertOrientFrm ) const;
    };
} // namespace objectpositioning

#endif
