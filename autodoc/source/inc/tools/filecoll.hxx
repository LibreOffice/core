/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: filecoll.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:10:22 $
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

#ifndef ADC_FILECOLL_HXX
#define ADC_FILECOLL_HXX

// USED SERVICES
    // BASE CLASSES
#include <autodoc/filecoli.hxx>
    // COMPONENTS
    // PARAMETERS



class FileCollector : public autodoc::FileCollector_Ifc
{
  public:
    // LIFECYCLE
                        FileCollector(
                            uintt               i_nRoughNrOfFiles = 0 );

    // OPERATIONS
    virtual uintt       AddFilesFrom(
                            const char *        i_sRootDir,
                            const char *        i_sFilter,
                            E_SearchMode        i_eSearchMode );
    virtual uintt       AddFile(
                            const char *        i_sFilePath );
    virtual void        EraseAll();

    // INQUIRY
    virtual const_iterator
                        Begin() const;
    virtual const_iterator
                        End() const;
    virtual uintt       Size() const;

  private:
    // DATA
    StringVector        aFoundFiles;
};


#endif

