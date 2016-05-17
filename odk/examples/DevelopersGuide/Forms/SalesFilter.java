/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  the BSD license.
 *
 *  Copyright 2000, 2010 Oracle and/or its affiliates.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of Sun Microsystems, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 *  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *************************************************************************/

import com.sun.star.uno.*;
import com.sun.star.beans.*;
import com.sun.star.form.*;
import com.sun.star.lang.*;
import com.sun.star.awt.*;

import java.util.Calendar;
import java.util.GregorianCalendar;

/**************************************************************************/
/** helper class for filtering the sales form
*/
class SalesFilter implements XActionListener, XPropertyChangeListener, XResetListener
{
    private DocumentHelper  m_aDocument;
    private XPropertySet        m_xSalesForm;

    private XPropertySet        m_xFilterList;
    private XPropertySet        m_xManualFilter;
    private XPropertySet        m_xApplyFilter;

    private boolean         m_bSettingsDirty;
    private boolean         m_bSettingDate;
    private boolean         m_bAdjustingFilterList;
    private short               m_nPreviousFilterIndex;
    private java.util.ArrayList    m_aFilterDates;

    /* ------------------------------------------------------------------ */
    public SalesFilter( DocumentHelper aDocument, XPropertySet xSalesForm,
        XPropertySet xFilterListBox, XPropertySet xManualFilterEdit, XPropertySet xStartFilterButton )
    {
        m_aFilterDates = new java.util.ArrayList();
        m_bSettingsDirty = false;
        m_bSettingDate = false;
        m_bAdjustingFilterList = false;
        m_nPreviousFilterIndex = -1;
        initFilterDates();


        // remember the components
        m_aDocument = aDocument;
        m_xSalesForm = xSalesForm;

        m_xFilterList = xFilterListBox;
        m_xManualFilter = xManualFilterEdit;
        m_xApplyFilter = xStartFilterButton;

        try
        {

            // init control models
            m_xFilterList.setPropertyValue( "Dropdown", Boolean.TRUE );
            m_xFilterList.setPropertyValue( "LineCount", Short.valueOf( (short)11 ) );
            m_xFilterList.setPropertyValue( "StringItemList", new String[] { "ever (means no filter)", "this morning", "1 week ago", "1 month ago", "1 year ago", "<other>" } );
            m_xFilterList.setPropertyValue( "DefaultSelection", new short[] { (short)0 } );

            m_xApplyFilter.setPropertyValue( "Label", "Apply Filter" );

            updateFilterControl();
            updateApplyButton();


            // add as listener to the events which require action

            // want to know about changed selection
            m_xFilterList.addPropertyChangeListener( "SelectedItems", this );
            m_xManualFilter.addPropertyChangeListener( "Date", this );

            // want to know about the date field being reset
            XReset xReset = UNO.queryReset( m_xManualFilter );
            xReset.addResetListener( this );

            // for the button, we can add to the control only, not to the model
            // - clicking a button is something which happens on the _control_.
            DocumentViewHelper aView = m_aDocument.getCurrentView();
            XButton xButton = aView.getFormControl( m_xApplyFilter, XButton.class );
            xButton.addActionListener( this );
        }
        catch ( com.sun.star.uno.Exception e )
        {
            System.out.println(e);
            e.printStackTrace();
        }
    }

    /* ==================================================================
       = helper
       ================================================================== */
    /* ------------------------------------------------------------------ */
    private void initFilterDates()
    {
        m_aFilterDates.clear();
        java.util.Date aNowAndHere = new java.util.Date();
        aNowAndHere.setHours( 0 );
        aNowAndHere.setMinutes( 0 );
        aNowAndHere.setSeconds( 0 );

        // for every list entry, we add a java.util.Date to m_aFilterDates indicating
        // since when the sales should be listed

        // ever
        m_aFilterDates.add( null );

        // this morning
        m_aFilterDates.add( aNowAndHere );

        // one week ago
        GregorianCalendar aCalendar = new GregorianCalendar( );
        aCalendar.setTime( aNowAndHere );
        aCalendar.add( Calendar.DATE, -7 );
        m_aFilterDates.add( aCalendar.getTime() );

        // one month ago
        aCalendar.setTime( aNowAndHere );
        aCalendar.add( Calendar.MONTH, -1 );
        m_aFilterDates.add( aCalendar.getTime() );

        // one year ago
        aCalendar.setTime( aNowAndHere );
        aCalendar.add( Calendar.YEAR, -1 );
        m_aFilterDates.add( aCalendar.getTime() );

        // the custom date
        m_aFilterDates.add( null );
    }

    /* ------------------------------------------------------------------ */
    /** translates a date from the AWT Toolkit format to a java.util.date, or
        vice versa.
    */
    private Object translateDate( Object aDate ) throws java.lang.Exception
    {
        Object aReturn = null;

        GregorianCalendar aCalDate = new GregorianCalendar();
        if ( aDate.getClass().equals( Class.forName( "java.util.Date" ) ) )
        {
            aCalDate.setTime( (java.util.Date)aDate );

            int nDate = aCalDate.get( Calendar.YEAR );
            nDate = nDate * 100 + aCalDate.get( Calendar.MONTH ) + 1;
            nDate = nDate * 100 + aCalDate.get( Calendar.DAY_OF_MONTH );

            aReturn = Integer.valueOf( nDate );
        }
        else if ( aDate.getClass().equals( Class.forName( "java.lang.Integer" ) ) )
        {
            int nToolkitDate = ((Integer)aDate).intValue();
            aCalDate.set( Calendar.DAY_OF_MONTH, ( nToolkitDate % 100 ) );
            nToolkitDate /= 100;
            aCalDate.set( Calendar.MONTH, ( nToolkitDate % 100 ) - 1 );
            nToolkitDate /= 100;
            aCalDate.set( Calendar.YEAR, ( nToolkitDate % 10000 ) );

            // default the time
            aCalDate.set( Calendar.HOUR_OF_DAY, 0 );
            aCalDate.set( Calendar.MINUTE, 0 );
            aCalDate.set( Calendar.SECOND, 0 );

            aReturn = aCalDate.getTime();
        }

        return aReturn;
    }

    /* ------------------------------------------------------------------ */
    /** translates the given date into the ODBC date notation, which then can be used
        for setting a filter at a row set
    */
    private String getOdbcDate( Object aDate ) throws java.lang.Exception
    {
        String sOdbcDate = "";
        if ( null != aDate )
        {
            if ( !aDate.getClass().equals( Class.forName( "java.util.Date" ) ) )
                aDate = translateDate( aDate );

            if ( aDate.getClass().equals( Class.forName( "java.util.Date" ) ) )
            {
                GregorianCalendar aCal = new GregorianCalendar();
                aCal.setTime( (java.util.Date)aDate );

                sOdbcDate += "{D '";
                sOdbcDate += (Integer.valueOf( aCal.get( Calendar.YEAR ) ) ).toString();
                sOdbcDate += "-";

                int nMonth = aCal.get( Calendar.MONTH ) + 1;
                if ( nMonth < 10 )
                    sOdbcDate += "0";
                sOdbcDate += (Integer.valueOf( nMonth ) ).toString();
                sOdbcDate += "-";

                int nDay = aCal.get( Calendar.DAY_OF_MONTH );
                if ( nDay < 10 )
                    sOdbcDate += "0";
                sOdbcDate += (Integer.valueOf( nDay ) ).toString();
                sOdbcDate += "'}";
            }
        }
        return sOdbcDate;
    }

    /* ------------------------------------------------------------------ */
    private void updateApplyButton()
    {
        try
        {
            m_xApplyFilter.setPropertyValue( "Enabled", Boolean.valueOf( m_bSettingsDirty ) );
        }
        catch ( com.sun.star.uno.Exception e )
        {
            System.out.println(e);
            e.printStackTrace();
        }
    }

    /* ------------------------------------------------------------------ */
    /** creates a normalized calendar object from the given java.util.Date
    */
    private GregorianCalendar getCalendarObject( java.util.Date aDate )
    {
        // the date part
        GregorianCalendar aReturn = null;
        if ( null != aDate )
        {
            aReturn = new GregorianCalendar( );
            aReturn.setTime( aDate );

            // normalize the time part
            aReturn.set( Calendar.HOUR_OF_DAY, 0 );
            aReturn.set( Calendar.MINUTE, 0 );
            aReturn.set( Calendar.SECOND, 0 );
        }

        return aReturn;
    }

    /* ------------------------------------------------------------------ */
    final private short getCurrentSelectedFilter( ) throws com.sun.star.uno.Exception
    {
        short[] aSelected = (short[])m_xFilterList.getPropertyValue( "SelectedItems" );
        if ( 0 < aSelected.length )
            return aSelected[0];
        return -1;
    }

    /* ------------------------------------------------------------------ */
    /** checks if the given filter index referes to the "<other>" entry which
        allows the user to manually enter a date
    */
    final private boolean isManualFilter( short nFilterIndex )
    {
        return ( 5 == nFilterIndex );
    }

    /* ------------------------------------------------------------------ */
    private void updateFilterControl()
    {
        try
        {
            if ( isManualFilter( m_nPreviousFilterIndex ) )
            {   // previously, the "custom" filter date was selected
                // -> remember the date entered
                Object aDate = translateDate( m_xManualFilter.getPropertyValue( "Date" ) );
                m_aFilterDates.set( m_nPreviousFilterIndex, aDate );
            }

            // check the current selection
            if ( !m_bAdjustingFilterList )
            {
                m_nPreviousFilterIndex = getCurrentSelectedFilter( );

                // custom filter?
                boolean bCustomFilter = isManualFilter( m_nPreviousFilterIndex );
                m_xManualFilter.setPropertyValue( "Enabled", Boolean.valueOf( bCustomFilter ) );
                if ( bCustomFilter )
                    m_aDocument.getCurrentView().grabControlFocus( m_xManualFilter );

                m_bSettingDate = true;
                Object aSelectedDateLimit = m_aFilterDates.get( m_nPreviousFilterIndex );
                if ( null != aSelectedDateLimit )
                {
                    // translate this date into one the AWT Toolkit understands
                    Integer aTKDate = (Integer)translateDate( aSelectedDateLimit );
                    m_xManualFilter.setPropertyValue( "Date", aTKDate );
                }
                else
                    m_xManualFilter.setPropertyValue( "Date", new Any( new Type(), null ) );
                m_bSettingDate = false;
            }
        }
        catch ( java.lang.Exception e )
        {
            System.out.println(e);
            e.printStackTrace();
        }
    }

    /* ------------------------------------------------------------------ */
    /** compares the date part of two calendars
        <p>For some strange reason I do not understand, GregorianCalendar.equals( GregorianCalendar )
        seems to always return false, as well as . Thus here is a method which compare two calendars,
        restricted to their date part</p>
    */
    private boolean equalDate( Calendar aLHS, Calendar aRHS )
    {
        if ( ( null == aLHS ) != ( null == aRHS ) )
            // only one of them is null
            return false;

        if ( null == aLHS )
            // both are null
            return true;

        return  ( aLHS.get( Calendar.YEAR ) == aRHS.get( Calendar.YEAR ) )
            &&  ( aLHS.get( Calendar.MONTH ) == aRHS.get( Calendar.MONTH ) )
            &&  ( aLHS.get( Calendar.DAY_OF_MONTH ) == aRHS.get( Calendar.DAY_OF_MONTH ) );
    }

    /* ------------------------------------------------------------------ */
    /** adds the current user filter to the list of date filters
        @return
            the index of the newly added date filter in the filter list
    */
    private short addCurrentFilter( ) throws java.lang.Exception
    {
        // the current string items
        String[] aOldFilterItems = (String[])m_xFilterList.getPropertyValue( "StringItemList" );

        // translate this into a vector - much more comfort to work with a vector than with an array ....
        java.util.ArrayList aFilterItems = new java.util.ArrayList();
        for ( int i=0; i<aOldFilterItems.length; ++i )
            aFilterItems.add( aOldFilterItems[i] );

        // the currently entered user defined filter date
        Object aDate = translateDate( m_xManualFilter.getPropertyValue( "Date" ) );
        GregorianCalendar aDateCal = getCalendarObject( (java.util.Date)aDate );
        // check if this date is already present in the list of user defined dates
        for ( int i=0; i<m_aFilterDates.size(); ++i )
        {
            if ( !isManualFilter( (short)i ) )  // do not compare with the manual filter
            {
                GregorianCalendar aCheckCal = getCalendarObject( (java.util.Date)m_aFilterDates.get( i ) );
                if ( equalDate( aDateCal, aCheckCal ) )
                    return (short)i;
            }
        }
        System.out.println( );

        if ( aFilterItems.size() > 10 ) // (6 standard items + 5 user defined items)
        {
            // the first (and thus oldest) user defined item
            aFilterItems.remove( 6 );
            // keep our date vector synchron
            m_aFilterDates.remove( 6 );
        }

        // add the current user defined filter
        aFilterItems.add( aDate.toString() );
        m_aFilterDates.add( aDate );

        // write back the string item list
        m_bAdjustingFilterList = true;
        String[] aNewFilterItems = new String[ aFilterItems.size() ];
        for ( int i=0; i<aFilterItems.size(); ++i )
            aNewFilterItems[i] = (String)aFilterItems.get( i );
        m_xFilterList.setPropertyValue( "StringItemList", aNewFilterItems );
        m_bAdjustingFilterList = false;

        return (short)(aNewFilterItems.length - 1 );
    }

    /* ------------------------------------------------------------------ */
    private void executeCurrentFilter()
    {
        try
        {
            // we keep the date field consistent with whatever the user chooses in the
            // list box, so just ask the field
            Object aDate = translateDate( m_xManualFilter.getPropertyValue( "Date" ) );
            String sOdbcDate = getOdbcDate( aDate );

            // if this filter was a manually entered filter, we add it to the filter list
            // box to allow quick-select it later on.
            if ( isManualFilter( getCurrentSelectedFilter() ) )
            {
                short nNewUserDefinedFilterPos = addCurrentFilter();
                m_xFilterList.setPropertyValue( "SelectedItems", new short[] { nNewUserDefinedFilterPos } );
            }

            // set this as filter on the form
            String sCompleteFilter = "";
            if ( ( null != sOdbcDate ) && ( 0 != sOdbcDate.length() ) )
            {
                sCompleteFilter = "SALEDATE >= ";
                sCompleteFilter += sOdbcDate;
            }
            m_xSalesForm.setPropertyValue( "Filter", sCompleteFilter );
            m_xSalesForm.setPropertyValue( "ApplyFilter", Boolean.TRUE );

            // and reload the form
            XLoadable xLoad = UnoRuntime.queryInterface(
                XLoadable.class, m_xSalesForm );
            xLoad.reload();

            m_aDocument.getCurrentView().grabControlFocus(  m_xFilterList );
        }
        catch ( java.lang.Exception e )
        {
            System.out.println(e);
            e.printStackTrace();
        }
    }

    /* ==================================================================
       = UNO callbacks
       ================================================================== */
    /* ------------------------------------------------------------------ */
    // XActionListener overridables
    /* ------------------------------------------------------------------ */
    public boolean approveReset( EventObject aEvent ) throws com.sun.star.uno.RuntimeException
    {
        return false;
            // do not allow the date field to be reset - it would set its content
            // to the current date
            // Note that another possible solution would be to wait for the resetted
            // event and correct the value there
    }

    /* ------------------------------------------------------------------ */
    public void resetted( EventObject aEvent ) throws com.sun.star.uno.RuntimeException
    {
    }

    /* ------------------------------------------------------------------ */
    // XActionListener overridables
    /* ------------------------------------------------------------------ */
    public void actionPerformed( ActionEvent aEvent ) throws com.sun.star.uno.RuntimeException
    {
        executeCurrentFilter();

        m_bSettingsDirty = false;
        updateApplyButton();
    }

    /* ------------------------------------------------------------------ */
    // XItemListener overridables
    /* ------------------------------------------------------------------ */
    public void propertyChange( PropertyChangeEvent aEvent ) throws com.sun.star.uno.RuntimeException
    {
        if ( aEvent.PropertyName.equals( "SelectedItems" ) )
        {
            updateFilterControl();

            m_bSettingsDirty = true;
            updateApplyButton();
        }
        else if ( aEvent.PropertyName.equals( "Date" ) && !m_bSettingDate )
        {
            m_bSettingsDirty = true;
            updateApplyButton();
        }
    }

    /* ------------------------------------------------------------------ */
    // XEventListener overridables
    /* ------------------------------------------------------------------ */
    public void disposing( EventObject aEvent )
    {
        // not interested in
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
