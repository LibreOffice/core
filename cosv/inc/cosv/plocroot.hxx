/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: plocroot.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 07:56:55 $
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

#ifndef CSV_PLOCROOT_HXX
#define CSV_PLOCROOT_HXX


// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
#include <cosv/string.hxx>
    // PARAMETERS
#include <cosv/csv_ostream.hxx>
#include <cosv/persist.hxx>


namespace csv
{

class bostream;


namespace ploc
{


class Root
{
  public:
    virtual             ~Root();

    static DYN Root *   Create_(
                            const char * &      o_sPathAfterRoot,
                            const char *        i_sPath,
                            const char *        i_sDelimiter = Delimiter() );

    virtual void        Get(                    /// Does not add a '\0' at the end,
                            ostream &           o_rPath ) const = 0;
    virtual void        Get(                    /// Does not add a '\0' at the end.
                            bostream &          so_rPath ) const = 0;
    virtual DYN Root *  CreateCopy() const = 0;
    virtual const char *
                        OwnDelimiter() const = 0;
};



}   // namespace ploc
}   // namespace csv



#endif



