/*************************************************************************
 *
 *  $RCSfile: WeakBase.java,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: jl $ $Date: 2002-04-11 13:39:51 $
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

package com.sun.star.lib.uno.helper;
import com.sun.star.uno.XWeak;
import com.sun.star.uno.XAdapter;
import com.sun.star.lang.XTypeProvider;
import com.sun.star.uno.Type;
import java.lang.reflect.Field;
import java.util.Vector;
import com.sun.star.uno.UnoRuntime;


/** This class can be used as the base class for UNO components. It implements the capability
 *  to be kept weak (com.sun.star.uno.XWeak) and it implements com.sun.star.lang.XTypeProvider
 *  which is necessary for using the component with StarBasic.
 */
public class WeakBase implements XWeak, XTypeProvider {
    private final boolean DEBUG= false;

    private final String UNO_INTERFACE_FIELD="UNOTYPEINFO";
    private final String UNO_INTERFACE_TYPE="[Lcom.sun.star.lib.uno.typeinfo.TypeInfo;";
    // Contains all WeakAdapter which have been created in this class
    // They have to be notified when this object dies
    private WeakAdapter m_adapter;

    // for XTypeProvider.getImplementationId
       static byte[] _implementationId;

     /** Method of XWeak. The returned XAdapter implementation can be used to keap
      * a weak reference to this object.
      * @return a com.sun.star.uno.XAdapter implementation.
      */
     synchronized public XAdapter queryAdapter() {
        if (m_adapter == null)
            m_adapter= new WeakAdapter(this);
         return m_adapter;
     }

     /** Override of Object.finalize. When there are no references to this object anymore
         then the garbage collector calls this method. Thereby causing the adapter object
         to be notified. The adapter, in turn, notifies all listeners (com.sun.star.uno.XReference)
      */
     protected void finalize() throws java.lang.Throwable {
        if (m_adapter != null)
            m_adapter.referentDying();
        super.finalize();
     }

     /** Method of XTypeProvider. It returns an array of Type objects which represent
      * all implemented UNO interfaces of this object.
      * @return Type objects of all implemented interfaces.
      */
     public Type[] getTypes() {
        Vector vec= new Vector();
        Class currentClass= getClass();
        do {
            Class interfaces[]= currentClass.getInterfaces();
            for(int i = 0; i < interfaces.length; ++ i) {
                // Test if it is a UNO interface, look for
                //    public static final com.sun.star.lib.uno.typeinfo.TypeInfo UNOTYPEINFO[]
                try{
                    Field unoField= interfaces[i].getField(UNO_INTERFACE_FIELD);
                    if (unoField.getType().getName().equals(UNO_INTERFACE_TYPE)) {
                        vec.add(new Type(interfaces[i]));
                    }
                }catch (NoSuchFieldException nfe) {
                }
            }
            // get the superclass the currentClass inherits from
            currentClass= currentClass.getSuperclass();
        } while (currentClass != null);

        Type types[]= new Type[vec.size()];
        for( int i= 0; i < types.length; i++)
           types[i]= (Type) vec.elementAt(i);

        return types;
     }

     /** Method of XTypeProvider. It provides an identifier that represents the set of UNO
      * interfaces implemented by this class. All instances of this class
      * which run in the same Java Virtual Machine return the same array. (This only works as long
      * the ClassLoader preserves the class even if no instance exist.)
      *@return identifier as array of bytes
      */
     synchronized public byte[] getImplementationId() {
         if (_implementationId == null) {
            int hash = hashCode();
            String sName= getClass().getName();
            byte[] arName= sName.getBytes();
            int nNameLength= arName.length;

            _implementationId= new byte[ 4 + nNameLength];
            _implementationId[0] = (byte)(hash & 0xff);
            _implementationId[1] = (byte)((hash >>> 8) & 0xff);
            _implementationId[2] = (byte)((hash >>> 16) & 0xff);
            _implementationId[3] = (byte)((hash >>>24) & 0xff);

            for (int i= 0; i < nNameLength; i++) {
                _implementationId[4 + i]= arName[i];
            }
        }
        return _implementationId;
     }
}
