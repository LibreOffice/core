/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: MNSProfileDirServiceProvider.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 01:48:13 $
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


#include "mozilla_nsprofiledirserviceprovider.h"

// Forward Declarations
class nsIAtom;

// --------------------------------------------------------------------------------------
// nsProfileDirServiceProvider - The nsIDirectoryServiceProvider implementation used for
// profile-relative file locations.
// --------------------------------------------------------------------------------------

class nsProfileDirServiceProvider: public nsIDirectoryServiceProvider
{
  NS_DECL_ISUPPORTS
  NS_DECL_NSIDIRECTORYSERVICEPROVIDER

  friend nsresult NS_NewProfileDirServiceProvider( nsProfileDirServiceProvider**);

public:

   virtual nsresult        SetProfileDir(nsIFile* aProfileDir);
   virtual nsresult         Register();
   virtual nsresult         Shutdown();

protected:
                           nsProfileDirServiceProvider();
   virtual                 ~nsProfileDirServiceProvider();

  nsresult                 Initialize();
  nsresult                 EnsureProfileFileExists(nsIFile* profileDir);
  nsresult                 UndefineFileLocations();

protected:

  // Atoms for file locations
  static nsIAtom*          sApp_PrefsDirectory50;
  static nsIAtom*          sApp_PreferencesFile50;
  static nsIAtom*          sApp_UserProfileDirectory50;

  nsCOMPtr<nsIFile>        mProfileDir;

};


// --------------------------------------------------------------------------------------

/**
 * Global method to create an instance of nsProfileDirServiceProvider
 *
 * @param aNotifyObservers    If true, will send out profile startup
 *                            notifications when the profile directory is set.
 *                            See nsIProfileChangeStatus.
 */

nsresult NS_NewProfileDirServiceProvider( nsProfileDirServiceProvider** aProvider);

