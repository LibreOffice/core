/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

package com.sun.star.wizards.common;

import java.util.Date;

import com.sun.star.beans.XPropertySet;
import com.sun.star.lang.Locale;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.util.NumberFormat;
import com.sun.star.util.XNumberFormatTypes;
import com.sun.star.util.XNumberFormats;
import com.sun.star.util.XNumberFormatsSupplier;
import com.sun.star.util.XNumberFormatter;


public class NumberFormatter
{

    public int iDateFormatKey = -1;
    public int iDateTimeFormatKey = -1;
    public int iNumberFormatKey = -1;
    public int iTextFormatKey = -1;
    public int iTimeFormatKey = -1;
    public int iLogicalFormatKey = -1;
    public long lDateCorrection;    
    public XNumberFormatter xNumberFormatter;
    public XNumberFormats xNumberFormats;
    public XNumberFormatTypes xNumberFormatTypes;
    public XPropertySet xNumberFormatSettings;
    private boolean bNullDateCorrectionIsDefined = false;
    private Locale aLocale; 
    
    
    public NumberFormatter(XMultiServiceFactory _xMSF, XNumberFormatsSupplier _xNumberFormatsSupplier, Locale _aLocale) throws Exception
    {
        aLocale = _aLocale;
        Object oNumberFormatter = _xMSF.createInstance("com.sun.star.util.NumberFormatter");
        xNumberFormats = _xNumberFormatsSupplier.getNumberFormats();
        xNumberFormatSettings = _xNumberFormatsSupplier.getNumberFormatSettings();
        xNumberFormatter = UnoRuntime.queryInterface(XNumberFormatter.class, oNumberFormatter);
        xNumberFormatter.attachNumberFormatsSupplier(_xNumberFormatsSupplier);  
        xNumberFormatTypes = UnoRuntime.queryInterface(XNumberFormatTypes.class, xNumberFormats);
        
    }
    
    public NumberFormatter(XNumberFormatsSupplier _xNumberFormatsSupplier, Locale _aLocale) throws Exception
    {
        aLocale = _aLocale;
        xNumberFormats = _xNumberFormatsSupplier.getNumberFormats();
        xNumberFormatSettings = _xNumberFormatsSupplier.getNumberFormatSettings();
        xNumberFormatTypes = UnoRuntime.queryInterface(XNumberFormatTypes.class, xNumberFormats);
    }

    
    /**
     * @param _xMSF
     * @param _xNumberFormatsSupplier
     * @return
     * @throws Exception
     * @deprecated
     * 
     */
    public static XNumberFormatter createNumberFormatter(XMultiServiceFactory _xMSF, XNumberFormatsSupplier _xNumberFormatsSupplier) throws Exception
    {
        Object oNumberFormatter = _xMSF.createInstance("com.sun.star.util.NumberFormatter");
        XNumberFormatter xNumberFormatter = UnoRuntime.queryInterface(XNumberFormatter.class, oNumberFormatter);
        xNumberFormatter.attachNumberFormatsSupplier(_xNumberFormatsSupplier);  
        return xNumberFormatter;
    }
    
    
    /**
     * gives a key to pass to a NumberFormat object. <br/>
     * example: <br/>
     * <pre>
     * XNumberFormatsSupplier nsf = (XNumberFormatsSupplier)UnoRuntime.queryInterface(...,document);
     * int key = Desktop.getNumberFormatterKey( nsf, ...star.i18n.NumberFormatIndex.DATE...);
     * XNumberFormatter nf = Desktop.createNumberFormatter(xmsf, nsf);
     * nf.convertNumberToString( key, 1972 );
     * </pre>
     * @param numberFormatsSupplier
     * @param type - a constant out of i18n.NumberFormatIndex enumeration.
     * @return a key to use with a util.NumberFormat instance.
     * 
     */
    public static int getNumberFormatterKey( Object numberFormatsSupplier, short type) 
    {
        Object numberFormatTypes = UnoRuntime.queryInterface(XNumberFormatsSupplier.class,numberFormatsSupplier).getNumberFormats();
        Locale l = new Locale();
        return UnoRuntime.queryInterface(XNumberFormatTypes.class,numberFormatTypes).getFormatIndex(type, l);
    }
    
    
    public String convertNumberToString(int _nkey, double _dblValue)
    {
        return xNumberFormatter.convertNumberToString(_nkey, _dblValue);
    }
    
    
    public static String convertNumberToString(XNumberFormatter _xNumberFormatter, int _nkey, double _dblValue)
    {
        return _xNumberFormatter.convertNumberToString(_nkey, _dblValue);
    }


    public double convertStringToNumber(int _nkey, String _sString)throws Exception
    {
        return xNumberFormatter.convertStringToNumber(_nkey, _sString);
    }

    
    /**
     * @param dateCorrection The lDateCorrection to set.
     */
    public void setNullDateCorrection(long dateCorrection) 
    {
        lDateCorrection = dateCorrection;
    }   
    
    
    public int defineNumberFormat(String _FormatString)
    {
        try 
        {
            int NewFormatKey = xNumberFormats.queryKey(_FormatString, aLocale, true);
            if (NewFormatKey == -1)
            {
                NewFormatKey = xNumberFormats.addNew(_FormatString, aLocale);
            }
            return NewFormatKey;
        }
        catch (Exception e)
        {
            e.printStackTrace(System.err);
            return -1;
        }
    }

    
    /**
     * returns a numberformat for a FormatString. 
     * @param _FormatString
     * @param _aLocale 
     * @return
     */ 
    public int defineNumberFormat(String _FormatString, Locale _aLocale)
    {
        try
        {
            int NewFormatKey = xNumberFormats.queryKey(_FormatString, _aLocale, true);
            if (NewFormatKey == -1)
            {
                NewFormatKey = xNumberFormats.addNew(_FormatString, _aLocale);
            }
            return NewFormatKey;
        }
        catch (Exception e)
        {
            e.printStackTrace(System.err);
            return -1;
        }
    }
    
    

    public void setNumberFormat(XInterface _xFormatObject, int _FormatKey, NumberFormatter _oNumberFormatter)
    {
        try
        {
            XPropertySet xNumberFormat = _oNumberFormatter.xNumberFormats.getByKey(_FormatKey);
            String FormatString = AnyConverter.toString(Helper.getUnoPropertyValue(xNumberFormat, "FormatString"));
            Locale oLocale = (Locale) Helper.getUnoPropertyValue(xNumberFormat, "Locale");
            int NewFormatKey = defineNumberFormat(FormatString, oLocale);
            XPropertySet xPSet = UnoRuntime.queryInterface(XPropertySet.class, _xFormatObject);
            if (xPSet.getPropertySetInfo().hasPropertyByName("FormatsSupplier"))
            {
                xPSet.setPropertyValue("FormatsSupplier", _oNumberFormatter.xNumberFormatter.getNumberFormatsSupplier());
            }
            if (xPSet.getPropertySetInfo().hasPropertyByName("NumberFormat"))
            {
                xPSet.setPropertyValue("NumberFormat", new Integer(NewFormatKey));
            }
            else if (xPSet.getPropertySetInfo().hasPropertyByName("FormatKey"))
            {
                xPSet.setPropertyValue("FormatKey", new Integer(NewFormatKey));
            }
            else
            {
                // TODO: throws a exception in a try catch environment, very helpful?
                throw new Exception();
            }
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.err);
        }
    }


    public long getNullDateCorrection()
    {
        if (!this.bNullDateCorrectionIsDefined)
        {
            com.sun.star.util.Date dNullDate = (com.sun.star.util.Date) Helper.getUnoStructValue(this.xNumberFormatSettings, "NullDate");
            long lNullDate = Helper.convertUnoDatetoInteger(dNullDate);
            java.util.Calendar oCal = java.util.Calendar.getInstance();
            oCal.set(1900, 1, 1);
            Date dTime = oCal.getTime();
            long lTime = dTime.getTime();
            long lDBNullDate = lTime / (3600 * 24000);
            lDateCorrection = lDBNullDate - lNullDate;
            return lDateCorrection;
        }
        else
        {
            return this.lDateCorrection;
        }
    }

    
    public int setBooleanReportDisplayNumberFormat()
    {
        String FormatString = "[=1]" + '"' + (char)9745 + '"' + ";[=0]" + '"' + (char)58480 + '"' + ";0";
        iLogicalFormatKey = xNumberFormats.queryKey(FormatString, aLocale, true);
        try
        {
            if (iLogicalFormatKey == -1)
            {
                iLogicalFormatKey = xNumberFormats.addNew(FormatString, aLocale);
            }
        }
        catch (Exception e)
        {         //MalformedNumberFormat
            e.printStackTrace();
            iLogicalFormatKey = xNumberFormatTypes.getStandardFormat(NumberFormat.LOGICAL, aLocale);                    
        }
        return iLogicalFormatKey;
    }   
    
    
    /**
     * @return Returns the iDateFormatKey.
     */
    public int getDateFormatKey()
    {
        if (iDateFormatKey == -1)
        {
            iDateFormatKey = xNumberFormatTypes.getStandardFormat(NumberFormat.DATE, aLocale);          
        }
        return iDateFormatKey;
    }
    /**
     * @return Returns the iDateTimeFormatKey.
     */
    public int getDateTimeFormatKey()
    {
        if (iDateTimeFormatKey == -1)
        {
            iDateTimeFormatKey = xNumberFormatTypes.getStandardFormat(NumberFormat.DATETIME, aLocale);
        }          
        return iDateTimeFormatKey;
    }
    /**
     * @return Returns the iLogicalFormatKey.
     */
    public int getLogicalFormatKey()
    {
        if (iLogicalFormatKey == -1)
        {
            iLogicalFormatKey = xNumberFormatTypes.getStandardFormat(NumberFormat.LOGICAL, aLocale);
        }            
        return iLogicalFormatKey;
    }
    /**
     * @return Returns the iNumberFormatKey.
     */
    public int getNumberFormatKey()
    {
        if (iNumberFormatKey == -1)
        {
            iNumberFormatKey = xNumberFormatTypes.getStandardFormat(NumberFormat.NUMBER, aLocale);
        }
        return iNumberFormatKey;
    }
    /**
     * @return Returns the iTextFormatKey.
     */
    public int getTextFormatKey()
    {
        if (iTextFormatKey == -1)
        {
            iTextFormatKey = xNumberFormatTypes.getStandardFormat(NumberFormat.TEXT, aLocale);
        }          
        return iTextFormatKey;
    }
    /**
     * @return Returns the iTimeFormatKey.
     */
    public int getTimeFormatKey()
    {
        if (iTimeFormatKey == -1)
        {
            iTimeFormatKey = xNumberFormatTypes.getStandardFormat(NumberFormat.TIME, aLocale);
        }      
        return iTimeFormatKey;
    }   
}
