/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ControlValidator.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:06:52 $
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

/** base class for components validating the content of form controls
 *
 * @author  fs@openoffice.org
 */
public abstract class ControlValidator implements com.sun.star.form.validation.XValidator
{

    /** Creates a new instance of ControlValidator */
    public ControlValidator()
    {
    }

    public void addValidityConstraintListener(com.sun.star.form.validation.XValidityConstraintListener xValidityConstraintListener)
    {
    }

    public void removeValidityConstraintListener(com.sun.star.form.validation.XValidityConstraintListener xValidityConstraintListener)
    {
    }

    protected boolean isVoid( Object Value )
    {
        try
        {
            com.sun.star.uno.Any anyValue = (com.sun.star.uno.Any)Value;
            return ( anyValue.getType().getTypeClass() == com.sun.star.uno.TypeClass.VOID );
        }
        catch( java.lang.ClassCastException e )
        {
        }
        return false;
    }
}
