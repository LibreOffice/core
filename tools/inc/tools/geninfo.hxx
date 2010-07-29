/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _BOOTSTRP_GENINFO_HXX
#define _BOOTSTRP_GENINFO_HXX

#include "tools/toolsdllapi.h"

#include <tools/string.hxx>
#include <tools/list.hxx>

// forwards
class GenericInformationList;

//
// class GenericInformation
//

/******************************************************************************
Purpose: holds generic informations and subinformations in a simple format
******************************************************************************/

class TOOLS_DLLPUBLIC GenericInformation : public ByteString    // the key is stored in base class
{
friend class GenericInformationList;    // can be child or/and parent
private:
    ByteString sValue;                  // holds value of data
    ByteString sComment;

    GenericInformationList *pInfoList;  // holds subinformations
    GenericInformationList *pParent;    // holds a pointer to parent list

    // methods
    void ListDeleted() { pParent = NULL; }  // allowed to be accessed
                                                // from friend class
                                                // GenericInformationList

public:
    GenericInformation( const ByteString &rKey, const ByteString &rValue,
                        GenericInformationList *pParentList = NULL,
                        GenericInformationList *pSubInfos = NULL );
    GenericInformation( const GenericInformation& rInf, sal_Bool bCopySubs = sal_True);

    ~GenericInformation();

  ByteString &GetValue() { return sValue; }
  void SetValue( const ByteString &rValue ) { sValue = rValue; }

  ByteString &GetComment() { return sComment; }
  void SetComment( const ByteString &rComment ) { sComment = rComment; }

    // this methods used to handle sub informations
    sal_Bool InsertSubInfo( GenericInformation *pInfo );
  // siehe GenericInformationList
  sal_Bool InsertSubInfo( const ByteString &rPathKey, const ByteString &rValue,
              sal_Bool bSearchByPath = sal_False, sal_Bool bNewPath = sal_False);
    void RemoveSubInfo( GenericInformation *pInfo, sal_Bool bDelete = sal_False );
  //  void RemoveSelf( sal_Bool bDelete = sal_False ); // loescht sich selbst aus der Parentliste
  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // bei bDelete = sal_True werden auch alle Sublisten UND DIE INFO SELBST geloescht.
  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    GenericInformation *GetSubInfo( ByteString &rKey, sal_Bool bSearchByPath = sal_False,
                    sal_Bool bCreatePath = sal_False );
                // path can be something like this: src370/drives/o:

    void SetSubList( GenericInformationList *pSubList )
        { pInfoList = pSubList; }
    GenericInformationList *GetSubList() { return pInfoList; }
};

/* diese Klasse bietet einen SemaphoreLock zum lesen (und nicht Schreiben)
 * oder schreiben (und nicht lesen)
 */
class GenericLockInformation : public GenericInformation
{
public:
GenericLockInformation( const ByteString &rKey, const ByteString &rValue,
            GenericInformationList *pParentList = NULL,
            GenericInformationList *pSubInfos = NULL )
  : GenericInformation( rKey, rValue, pParentList, pSubInfos),
    aLockState( read ), nLockKey( 0 ) {};
  //~GenericLockInformation();

  /* bietet einen Lockmechanismus fuer exclusive Zugriffe
   *
   * -"writeonly" wird angesprochen, wenn von der Wurzel ausgehend
   *  ein Item veraendert werden soll. In der Zeit kann die Liste nicht
   *  gelesen werden, womit keine Inconsistenzen entstehen koennen.
   *
   * -"read" wird zum Normalen lesen der Infos benutzt, 90% der Betriebszeit.
   *  waerenddessen kann nicht geschrieben werden -> writeonly Lock.
   *  Ist fuer den atomaren(nicht unterbrochenen) Lesezugriff gedacht
   *
   * -"readonly" wird zum niederschreiben des Teilbaums benutzt, was schon mal
   *  10 Minuten dauern kann. In der Zeit kann kein Writeonlylock gesetzt
   *  werden, aber ein rescedule. Damit koennen andere Aktionen asynchron ausgefuert
   *  werden, aber die Datensicherheit bleibt gewahrt
   *
   * Zustandsaenderung: writeonly <-> read <-> readonly
   *
   * nLockKey ist zum verschluesseln des LockZugriffs mit einem 32bit Wort vorgesehen.
   * ist der Schluessel nicht null, so kann nur mit dem Schluessel in
   * die Baumstruktur geschrieben werden.
   * ist der Schluessel nLockKey Null, dann kann jeder Schreiben und die Locks loesen
   */
  enum LockState{ writeonly, read, readonly };

  /* der Schreibschutz darf nur aktiviert werden, wenn
   * der Status auf Lesen steht
   */
  sal_Bool SetWriteLock(sal_uInt32 nKey = 0) { return ((read==aLockState) &&
                           (aLockState=writeonly, nLockKey=nKey, sal_True)); }
  /* Schreibschutz darf nur geloest werden, wenn
   * der Schreibschutz drin ist, und
   * entweder der LockKey Null ist(Generalschluessel) oder der Key zum LockKey passt
   */
  sal_Bool ReleaseWriteLock(sal_uInt32 nKey = 0) { return ((writeonly==aLockState) &&
                           (!nLockKey||nKey==nLockKey) &&
                           (aLockState=read, nLockKey=0, sal_True)); } // setzt den zustand auf "read"
  sal_Bool SetReadLock(sal_uInt32 nKey = 0) { return ((read==aLockState) &&
                          (aLockState=readonly, nLockKey=nKey, sal_True)); }
  sal_Bool ReleaseReadLock(sal_uInt32 nKey = 0) { return ((readonly==aLockState) &&
                          (!nLockKey||nKey==nLockKey) &&
                          (aLockState=read, nLockKey=0, sal_True)); } // setzt den zustand auf "read"

  LockState GetLockState() const { return aLockState; }
  sal_Bool IsWriteLocked() const { return (writeonly==aLockState); }
  sal_Bool IsReadLocked() const { return (readonly==aLockState); }
  sal_Bool IsNotLocked() const { return (read==aLockState); }
  sal_Bool IsLocker( sal_uInt32 nKey ) { return (nKey==nLockKey || !nLockKey); }

  /* wenn der Schreibschutz aktiviert wurde,
   * und bei vorhandenem Schreibschutz die Keys stimmen
   * rufe die Parentmethode auf */
  sal_Bool InsertSubInfo( GenericInformation *pInfo, sal_uInt32 nKey = 0 ) {
    return ((writeonly==aLockState) &&
        (!nLockKey || nKey==nLockKey) &&
        (GenericInformation::InsertSubInfo( pInfo ), sal_True)); }

  sal_Bool InsertSubInfo( const ByteString &rPathKey, const ByteString &rValue, sal_uInt32 nKey = 0,
              sal_Bool bSearchByPath = sal_False, sal_Bool bNewPath = sal_False) {
    return ((writeonly==aLockState) &&
        (!nLockKey || nKey==nLockKey) &&
        (GenericInformation::InsertSubInfo( rPathKey, rValue, bSearchByPath, bNewPath ), sal_True)); }
  /* 29.jan.98: erweiterung um lesemoeglichkeit vom Lockclienten */
  GenericInformation *GetSubInfo( ByteString &rKey, sal_Bool bSearchByPath = sal_False,
                  sal_Bool bCreatePath = sal_False, sal_uInt32 nKey = 0 ) {
    if (writeonly==aLockState && nLockKey && nKey!=nLockKey )
      return NULL;
    return GenericInformation::GetSubInfo(rKey, bSearchByPath, bCreatePath); }

  //  TYPEINFO();
private:

  LockState aLockState;
  sal_uInt32    nLockKey;
};

//
// class GenericInformationList
//

/******************************************************************************
Purpose: holds set of generic informations in a sorted list
******************************************************************************/

DECLARE_LIST( GenericInformationList_Impl, GenericInformation * )

class TOOLS_DLLPUBLIC GenericInformationList : public GenericInformationList_Impl
{
private:
    GenericInformation *pOwner;         // holds parent of this list

protected:
    // methods
    sal_uIntPtr InsertSorted( GenericInformation *pInfo, sal_Bool bOverwrite,
                            sal_uIntPtr nStart, sal_uIntPtr nEnd );
    GenericInformation *Search( sal_uIntPtr &rPos, ByteString sKey,
                            sal_uIntPtr nStart, sal_uIntPtr nEnd );

public:
    GenericInformationList( GenericInformation *pParent = NULL );
    GenericInformationList(const GenericInformationList& rList, GenericInformation *pParent = NULL);
    ~GenericInformationList();

    // this methods used to handle the informations using binary search
    GenericInformation *GetInfo( ByteString &rKey, sal_Bool bSearchByPath = sal_False,
                     sal_Bool bCreatePath = sal_False );
  /* path can be something like this: src370/drives/o:
   * bCreatePath will create the neccecary paths to the GI */

    sal_Bool InsertInfo( GenericInformation *pInfo, sal_Bool bOverwrite = sal_True );
  /* legt eine GenericInformation im Baum an mit Key-Value
   * wenn bNewPath gesetzt, wird der nichtexistente Teil des Pfades neu kreiert
   * wenn bNewPath nicht gesetzt ist und ein Teil des Pfades nicht vorhanden ist,
   * gibt die Methode sal_False zurueck.*/
  sal_Bool InsertInfo( const ByteString &rPathKey, const ByteString &rValue,
           sal_Bool bSearchByPath = sal_False, sal_Bool bNewPath = sal_False);
    void RemoveInfo( GenericInformation *pInfo, sal_Bool bDelete = sal_False );

    GenericInformation* SetOwner( GenericInformation *pNewOwner );

};

#endif

