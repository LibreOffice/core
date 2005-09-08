/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cfgfile.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 04:47:33 $
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


// -----------------------------------------------------------------------------
// ------------------------------------ main ------------------------------------
// -----------------------------------------------------------------------------

/*
  OUString operator+(const OUString &a, const OUString &b)
  {
  OUString c = a;
  c += b;
  return c;
  }
*/

namespace configmgr
{
    void simpleMappingTest();

    void importTest();
    void exportTest();

    void hierarchyTest();
    //void simpleTest();
    void speedTest();
    void stringTest();
    void classTest();
    void hash_test();
    void testRefs();
    void ConfigName();

    void oslTest();
}



#if (defined UNX) || (defined OS2)
int main( int argc, char * argv[] )
#else
    int _cdecl main( int argc, char * argv[] )
#endif
{

//  configmgr::hierarchyTest();


//  configmgr::importTest();
//  configmgr::exportTest();
//  configmgr::speedTest();
//  configmgr::simpleTest();

//  configmgr::simpleMappingTest();
//  configmgr::stringTest();
//  configmgr::classTest();

//  configmgr::hash_test();

    // configmgr::testRefs();
    // configmgr::ConfigName();

    configmgr::oslTest();
    return 0;
}


