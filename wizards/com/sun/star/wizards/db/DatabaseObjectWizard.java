/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.sun.star.wizards.db;

import com.sun.star.beans.PropertyValue;
import com.sun.star.container.NoSuchElementException;
import com.sun.star.frame.XController;
import com.sun.star.frame.XFrame;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sdb.application.XDatabaseDocumentUI;
import com.sun.star.sdbc.SQLException;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.wizards.common.Desktop;
import com.sun.star.wizards.common.NamedValueCollection;
import com.sun.star.wizards.ui.WizardDialog;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 * is a base class for a wizard creating a database object
 * @author frank.schoenheit@sun.com
 */
public abstract class DatabaseObjectWizard extends WizardDialog
{
    protected final PropertyValue[]     m_wizardContext;
    protected final XDatabaseDocumentUI m_docUI;
    protected final XFrame              m_frame;

    protected DatabaseObjectWizard( final XMultiServiceFactory i_orb, final int i_helpIDBase, final PropertyValue[] i_wizardContext )
    {
        super( i_orb, i_helpIDBase );
        m_wizardContext = i_wizardContext;

        final NamedValueCollection wizardContext = new NamedValueCollection( m_wizardContext );
        m_docUI = wizardContext.queryOrDefault( "DocumentUI", (XDatabaseDocumentUI)null, XDatabaseDocumentUI.class );

        if ( m_docUI != null )
        {
            XController docController = UnoRuntime.queryInterface( XController.class, m_docUI );
            m_frame = docController.getFrame();
        }
        else
        {
            XFrame parentFrame = wizardContext.queryOrDefault( "ParentFrame", (XFrame)null, XFrame.class );
            if ( parentFrame != null )
                m_frame = parentFrame;
            else
                m_frame = Desktop.getActiveFrame( xMSF );
        }
    }

    protected final void loadSubComponent( final int i_type, final String i_name, final boolean i_forEditing )
    {
        try
        {
            if ( m_docUI != null )
                m_docUI.loadComponent( i_type, i_name, i_forEditing );
        }
        catch ( IllegalArgumentException ex )
        {
            Logger.getLogger( this.getClass().getName() ).log( Level.SEVERE, null, ex );
        }
        catch ( NoSuchElementException ex )
        {
            Logger.getLogger( this.getClass().getName() ).log( Level.SEVERE, null, ex );
        }
        catch ( SQLException ex )
        {
            Logger.getLogger( this.getClass().getName() ).log( Level.SEVERE, null, ex );
        }
    }
}
