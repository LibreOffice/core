import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.border.*;
import java.util.Vector;
import com.sun.star.script.framework.runtime.XScriptContext;


public class PostNewsgroup extends JFrame
{

    // Post to newsgroup objects
    private NewsGroup[] subscribedNewsgroups = null;
    private XScriptContext xscriptcontext = null;

    private final int FRAMEX = 300;
    private final int FRAMEY = 300;
    private final int TEXTBOXWIDTH = 300;
    private final int TEXTBOXHEIGHT = 24;
    private final int TEXTAREAHEIGHT = 70;
    private final int BUTTONWIDTH = 80;
    private final int BUTTONHEIGHT = 30;

    private PostNewsgroup window = null;
    private JComboBox newsgroupComboBox = null;
    private JTextField hostTextField = null;
    private JTextField replyTextField = null;
    private JTextField subjectTextField = null;
    private JTextArea commentTextArea = null;
    private JRadioButton officeHtmlButton = null;
    private JRadioButton officeButton = null;
    private JRadioButton htmlButton = null;
    private JButton postButton = null;
    private JButton cancelButton = null;

    // JFrame for launch progress dialog
    private StatusWindow statusWindow = null;
    private String statusLine = "";

    // Tool tip text
    private final String newsgroupText = "Newsgroup name";
    private final String hostText = "Newsgroup host/server name";
    private final String replyText = "Email address to reply to";
    private final String subjectText = "Subject title for the mail";
    private final String commentText = "Additional comment on mail";
    private final String officeHtmlText = "Post as both Office and HTML attachments";
    private final String officeText = "Post as Office attachment only";
    private final String htmlText = "Post as HTML attachment only";
    private final String postText = "Post to newsgroup";
    private final String cancelText = "Cancel post to newsgroup";


    public void post( XScriptContext xsc )
    {
        xscriptcontext = xsc;
        window = this;

        // create mailcap and mimetypes files (fix for classloader problem)
        MimeConfiguration.createFiles( xscriptcontext );

        this.setTitle( "Post Document To Newsgroup" );
        this.setLocation( FRAMEX, FRAMEY );

        this.addFocusListener( new FocusAdapter()
        {
            public void focusGained( FocusEvent event )
            {
                System.out.println( "Focus gained" );
                window.update( window.getGraphics() );
            }

            public void focusLost( FocusEvent event )
                        {
                System.out.println( "Focus lost" );
                        }
        });

        Container container = getContentPane();
        container.setLayout( new GridBagLayout() );;
        GridBagConstraints constraints = new GridBagConstraints();
        constraints.fill = GridBagConstraints.BOTH;

        JPanel labelPanel = constructLabelPanel();
        JPanel textPanel = constructTextPanel();
        JPanel optionPanel = constructOptionPanel();
        JPanel buttonPanel = constructButtonPanel();

        constraints.gridx = 0;
        constraints.gridy = 0;
        constraints.gridwidth = 1;
        constraints.gridheight = 3;
        constraints.insets = new Insets( 15, 15, 5, 5 );
        container.add( labelPanel, constraints );

        constraints.gridx = 1;
        constraints.gridy = 0;
        constraints.gridwidth = 4;
        constraints.gridheight = 3;
        constraints.insets = new Insets( 15, 5, 5, 15 );
        container.add( textPanel, constraints );

        constraints.gridx = 0;
        constraints.gridy = 3;
        constraints.gridwidth = 5;
        constraints.gridheight = 1;
        constraints.insets = new Insets( 5, 15, 5, 15 );
        container.add( optionPanel, constraints );

        constraints.gridx = 0;
        constraints.gridy = 4;
        constraints.gridwidth = 5;
        constraints.gridheight = 1;
        constraints.insets = new Insets( 5, 5, 5, 5 );
        container.add( buttonPanel, constraints );

        this.pack();
        this.setResizable( false );
        this.setVisible( true );
    }


    private JPanel constructLabelPanel()
    {
        JLabel newsgroupLabel = new JLabel( "Newsgroup:" );
        JLabel hostLabel = new JLabel( "Host:" );
        JLabel replyLabel = new JLabel( "Reply:" );
        JLabel subjectLabel = new JLabel( "Subject:" );
        JLabel commentLabel = new JLabel( "Comment:" );

        newsgroupLabel.setToolTipText( newsgroupText );
        hostLabel.setToolTipText( hostText );
        replyLabel.setToolTipText( replyText );
        subjectLabel.setToolTipText( subjectText );
        commentLabel.setToolTipText( commentText );

        JPanel newsgroupPanel = new JPanel();
        newsgroupPanel.setLayout( new BorderLayout() );
        newsgroupPanel.add( newsgroupLabel, "West" );
        JPanel hostPanel = new JPanel();
        hostPanel.setLayout( new BorderLayout() );
        hostPanel.add( hostLabel, "West" );
        JPanel replyPanel = new JPanel();
        replyPanel.setLayout( new BorderLayout() );
        replyPanel.add( replyLabel, "West" );
        JPanel subjectPanel = new JPanel();
        subjectPanel.setLayout( new BorderLayout() );
        subjectPanel.add( subjectLabel, "West" );
        JPanel commentPanel = new JPanel();
        commentPanel.setLayout( new BorderLayout() );
        commentPanel.add( commentLabel, "West" );
        JPanel emptyPanel = new JPanel();

        final int labelWidth = 80;
        newsgroupPanel.setPreferredSize( new Dimension( labelWidth, TEXTBOXHEIGHT ) );
        hostPanel.setPreferredSize( new Dimension( labelWidth, TEXTBOXHEIGHT ) );
        replyPanel.setPreferredSize( new Dimension( labelWidth, TEXTBOXHEIGHT ) );
        subjectPanel.setPreferredSize( new Dimension( labelWidth, TEXTBOXHEIGHT ) );
        commentPanel.setPreferredSize( new Dimension( labelWidth, TEXTBOXHEIGHT ) );

        JPanel panel = new JPanel();
        panel.setLayout( new GridBagLayout() );
        GridBagConstraints constraints = new GridBagConstraints();
        constraints.fill = GridBagConstraints.BOTH;
        constraints.insets = new Insets( 5, 5, 5, 5 );

        constraints.gridx = 0;
        constraints.gridy = 0;
        constraints.gridwidth = 1;
        constraints.gridheight = 1;
        constraints.weightx = constraints.weighty = 0.0;
        panel.add( newsgroupPanel, constraints );

        constraints.gridx = 0;
        constraints.gridy = 1;
        constraints.gridwidth = 1;
        constraints.gridheight = 1;
        panel.add( hostPanel, constraints );

        constraints.gridx = 0;
        constraints.gridy = 2;
        constraints.gridwidth = 1;
        constraints.gridheight = 1;
        panel.add( replyPanel, constraints );

        constraints.gridx = 0;
        constraints.gridy = 3;
        constraints.gridwidth = 1;
        constraints.gridheight = 1;
        panel.add( subjectPanel, constraints );

        constraints.gridx = 0;
        constraints.gridy = 4;
        constraints.gridwidth = 1;
        constraints.gridheight = 1;
        panel.add( commentPanel, constraints );

        constraints.gridx = 0;
        constraints.gridy = 5;
        constraints.gridwidth = 1;
        constraints.gridheight = 1;
        constraints.weightx = constraints.weighty = 1.0;
        panel.add( emptyPanel, constraints );

        return panel;
    }


    private JPanel constructTextPanel()
    {
        hostTextField = new JTextField();
                hostTextField.setPreferredSize( new Dimension( TEXTBOXWIDTH, TEXTBOXHEIGHT ) );
                hostTextField.setToolTipText( hostText );
        hostTextField.setBorder( new EtchedBorder() );

        //optionPanel.setBorder( new TitledBorder( new EtchedBorder(), "Document Format" ) );
        newsgroupComboBox = getNewsgroupCombo();

        replyTextField = new JTextField();
        replyTextField.setPreferredSize( new Dimension( TEXTBOXWIDTH, TEXTBOXHEIGHT ) );
        replyTextField.setToolTipText( replyText );
        replyTextField.setBorder( new EtchedBorder() );

        subjectTextField = new JTextField();
        subjectTextField.setPreferredSize( new Dimension( TEXTBOXWIDTH, TEXTBOXHEIGHT ) );
        subjectTextField.setToolTipText( subjectText );
        subjectTextField.setBorder( new EtchedBorder() );

        commentTextArea = new JTextArea();
        commentTextArea.setPreferredSize( new Dimension( TEXTBOXWIDTH, TEXTAREAHEIGHT ) );
        commentTextArea.setToolTipText( commentText );
        commentTextArea.setBorder( new EtchedBorder() );

        JPanel panel = new JPanel();
        panel.setLayout( new GridBagLayout() );
        GridBagConstraints constraints = new GridBagConstraints();
        constraints.fill = GridBagConstraints.BOTH;
        constraints.insets = new Insets( 5, 5, 5, 5 );

        constraints.gridx = 0;
        constraints.gridy = 0;
        constraints.gridwidth = 1;
        constraints.gridheight = 1;
        panel.add( newsgroupComboBox, constraints );

        constraints.gridx = 0;
        constraints.gridy = 1;
        constraints.gridwidth = 1;
        constraints.gridheight = 1;
        panel.add( hostTextField, constraints );

        constraints.gridx = 0;
        constraints.gridy = 2;
        constraints.gridwidth = 1;
        constraints.gridheight = 1;
        panel.add( replyTextField, constraints );

        constraints.gridx = 0;
        constraints.gridy = 3;
        constraints.gridwidth = 1;
        constraints.gridheight = 1;
        panel.add( subjectTextField, constraints );

        constraints.gridx = 0;
        constraints.gridy = 4;
        constraints.gridwidth = 1;
        constraints.gridheight = 2;
        panel.add( commentTextArea, constraints );

        return panel;
    }


    private JComboBox getNewsgroupCombo()
    {
        newsgroupComboBox = new JComboBox();
        //newsgroupComboBox.setBorder( new EtchedBorder() );

           newsgroupComboBox.addActionListener(new ActionListener()
           {
                public void actionPerformed(ActionEvent e)
                {
                // when newsgroup is selected
                if( subscribedNewsgroups != null )
                {
                    int position = newsgroupComboBox.getSelectedIndex();
                    if( position != -1 )
                    {
                        hostTextField.setText( subscribedNewsgroups[ position ].getHostName() );
                        newsgroupComboBox.setToolTipText( "Newsgroup name: " + subscribedNewsgroups[ position ].getNewsgroupName() + "  (Host name: " + subscribedNewsgroups[ position ].getHostName() + ")" );
                    }
                }
            }
        });

        NewsGroup groupToSend = null;
        SubscribedNewsgroups newsgroups = new SubscribedNewsgroups();
        subscribedNewsgroups = newsgroups.getNewsGroups();

        // Test for no .mozilla or no subscribed newsgroups
        // subscribedNewsgroups = null;

        if( subscribedNewsgroups == null )
        {
            //System.out.println( "Couldn't find any subscibed newsgroups in .mozilla" );
            JOptionPane.showMessageDialog( window, "No subscribed newsgroups found in mozilla/netscape profile \nPlease enter newsgroup and host name",
                            "Newsgroups Information", JOptionPane.INFORMATION_MESSAGE );
        }
        else
        {
            // Copy all newsgroups into a vector for comparison
            // Alter entries (to include host name) if duplication is found
            Vector vector = new Vector( subscribedNewsgroups.length );
            for(int i=0; i < subscribedNewsgroups.length; i++ )
            {
                vector.add( subscribedNewsgroups[i].getNewsgroupName() );
            }
            // Compare and alter
            for(int i=0; i < subscribedNewsgroups.length; i++ )
            {
                // check if combo box already has a newsgroup with same name
                // then add host name to differentiate
                for(int j=0; j < subscribedNewsgroups.length; j++ )
                {
                    if( j != i && subscribedNewsgroups[j].getNewsgroupName().equalsIgnoreCase( subscribedNewsgroups[i].getNewsgroupName() ) )
                    {
                        vector.set( j, subscribedNewsgroups[j].getNewsgroupName() + "  (" + subscribedNewsgroups[j].getHostName() + ")" );
                        vector.set( i, subscribedNewsgroups[i].getNewsgroupName() + "  (" + subscribedNewsgroups[i].getHostName() + ")" );
                    }
                }
            }
            // Copy converted newsgroups from vector to combo box
            for(int i=0; i < subscribedNewsgroups.length; i++ )
            {
                newsgroupComboBox.addItem( vector.elementAt(i) );
            }
        }// else

        newsgroupComboBox.setPreferredSize( new Dimension( TEXTBOXWIDTH, TEXTBOXHEIGHT ) );
        newsgroupComboBox.setEditable( true );

        return newsgroupComboBox;
    }



    private JPanel constructOptionPanel()
    {
        officeHtmlButton = new JRadioButton( "Office and HTML", true );
        officeHtmlButton.setToolTipText( officeHtmlText );

        officeButton = new JRadioButton( "Office" );
        officeButton.setToolTipText( officeText );

        htmlButton = new JRadioButton( "HTML" );
        htmlButton.setToolTipText( htmlText );

        JRadioButton[] rbuttons = { officeHtmlButton, officeButton, htmlButton };
        ButtonGroup radioButtonGroup = new ButtonGroup();
        for( int i=0; i < rbuttons.length; i++ )
        {
            radioButtonGroup.add( rbuttons[i] );
        }

        JPanel optionPanel = new JPanel();
        //optionPanel.setLayout( new GridLayout( 1, 3, 20, 0 ) );
        optionPanel.setBorder( new TitledBorder( new EtchedBorder(), "Document Format" ) );
        optionPanel.setLayout( new GridBagLayout() );
        GridBagConstraints constraints = new GridBagConstraints();
        constraints.fill = GridBagConstraints.BOTH;

        constraints.gridx = 0;
                constraints.gridy = 0;
                constraints.gridwidth = 1;
                constraints.gridheight = 1;
        constraints.insets = new Insets( 5, 5, 5, 30 );
                optionPanel.add( officeHtmlButton, constraints );

        constraints.gridx = 1;
                constraints.gridy = 0;
                constraints.gridwidth = 1;
                constraints.gridheight = 1;
        constraints.insets = new Insets( 5, 20, 5, 30 );
                optionPanel.add( officeButton, constraints );

        constraints.gridx = 2;
                constraints.gridy = 0;
                constraints.gridwidth = 1;
                constraints.gridheight = 1;
        constraints.insets = new Insets( 5, 20, 5, 5 );
                optionPanel.add( htmlButton, constraints );

                return optionPanel;
    }



    public boolean sendingActions()
    {
            // posting actions
            // Validate the data
            if( isValidData() )
            {
                // Create status window
                StatusWindow statusWindow = new StatusWindow( window,  "Posting to Newsgroup", FRAMEX, FRAMEY );

                statusWindow.setVisible( true );
                //statusWindow.requestFocusInWindow();
                statusLine = "Ready to send...";
                statusWindow.setStatus( 0, statusLine );

                // Get the boolean values for HTML/Office document
                // params: ( XScriptContext, StatusWindow, html document, office document )

                boolean html = false;
                boolean office = false;
                if( officeHtmlButton.isSelected() ) { html = true; office = true; }
                if( officeButton.isSelected() ) { office = true; html = false; }
                if( htmlButton.isSelected() ) { html = true; office = false; }

                OfficeAttachment officeAttach = new OfficeAttachment( xscriptcontext, statusWindow, html, office );

                statusLine = "Getting user input";
                statusWindow.setStatus( 2, statusLine );
                // Get replyto, subject, comment from textboxes
                String replyto = replyTextField.getText();
                String subject = subjectTextField.getText();
                String comment = commentTextArea.getText();

                // Get newsgroup from combo box (corresponding position)
                String host = "";
                String group = "";
                int position = newsgroupComboBox.getSelectedIndex();
                if( subscribedNewsgroups == null || position == -1 )
                {
                    host = hostTextField.getText();
                    group = newsgroupComboBox.getSelectedItem().toString();
                }
                else
                {
                    //int position = newsgroupComboBox.getSelectedIndex();
                    host = subscribedNewsgroups[ position ].getHostName();
                    group = subscribedNewsgroups[ position ].getNewsgroupName();
                }

                statusLine = "Creating sender object";
                statusWindow.setStatus( 3, statusLine );
                Sender sender = new Sender( statusWindow, officeAttach, replyto, subject, comment, host, group );
                if( !sender.sendMail() )
                {
                    //System.out.println( "Should end here (?)" );
                    statusWindow.enableCancelButton( true );
                    officeAttach.cleanUpOnError();
                    return false;
                }

                statusLine = "Send is complete";
                statusWindow.setStatus( 14, statusLine );
            }
            else
            {
                //System.out.println( "Non valid data" );
                return false;
            }
            return true;
    }


    private JPanel constructButtonPanel()
    {
        Action postAction = new AbstractAction() {
            public void actionPerformed( ActionEvent event ) {
                // posting actions
                sendingActions();
            }// actionPerformed
        };

        Action cancelAction = new AbstractAction() {
                        public void actionPerformed( ActionEvent event ) {
                                // cancelling actions
                window.dispose();
                        }
                };

        postButton = new JButton();
        postButton.setAction( postAction );
        postButton.setToolTipText( postText );
        postButton.setText( "Post" );
        postButton.setPreferredSize( new Dimension( BUTTONWIDTH + 20, BUTTONHEIGHT ) );

        cancelButton = new JButton();
        cancelButton.setAction( cancelAction );
        cancelButton.setToolTipText( cancelText );
        cancelButton.setText( "Cancel" );
        cancelButton.setPreferredSize( new Dimension( BUTTONWIDTH + 20, BUTTONHEIGHT ) );

        JSeparator sep = new JSeparator( SwingConstants.HORIZONTAL );

        JPanel buttonPanel = new JPanel();
        buttonPanel.setLayout( new GridBagLayout() );
        GridBagConstraints constraints = new GridBagConstraints();
        constraints.fill = GridBagConstraints.BOTH;
        constraints.insets = new Insets( 5, 5, 5, 5 );

        JPanel emptyPanel1 = new JPanel();
        emptyPanel1.setPreferredSize( new Dimension( BUTTONWIDTH, BUTTONHEIGHT ) );

        JPanel emptyPanel2 = new JPanel();
        emptyPanel2.setPreferredSize( new Dimension( BUTTONWIDTH, BUTTONHEIGHT ) );

        constraints.gridx = 0;
                constraints.gridy = 0;
                constraints.gridwidth = 4;
                constraints.gridheight = 1;
                buttonPanel.add( sep, constraints );

        constraints.gridx = 0;
                constraints.gridy = 1;
                constraints.gridwidth = 1;
                constraints.gridheight = 1;
                buttonPanel.add( emptyPanel1, constraints );

                constraints.gridx = 1;
                constraints.gridy = 1;
                constraints.gridwidth = 1;
                constraints.gridheight = 1;
                buttonPanel.add( emptyPanel2, constraints );

        constraints.gridx = 2;
        constraints.gridy = 1;
        constraints.gridwidth = 1;
        constraints.gridheight = 1;
        buttonPanel.add( postButton, constraints );

        constraints.gridx = 3;
        constraints.gridy = 1;
        constraints.gridwidth = 1;
        constraints.gridheight = 1;
        constraints.insets = new Insets( 5, 5, 5, 0 );
        buttonPanel.add( cancelButton, constraints );

                return buttonPanel;
    }


    public void enableButtons( boolean enable )
    {
        if( enable )
        {
            postButton.setEnabled( true );
            cancelButton.setEnabled( true );
        }
        else
        {
            postButton.setEnabled( false );
            cancelButton.setEnabled( false );
        }
    }


    private boolean isValidData()
    {
        // newsgroupComboBox must not be blank (format? dots and whitespace)
        String newsgroupString = "";
        int position = newsgroupComboBox.getSelectedIndex();
        if( subscribedNewsgroups == null || position == -1 )
        {
            newsgroupString = newsgroupComboBox.getSelectedItem().toString();
        }
        else
        {
            //int position = newsgroupComboBox.getSelectedIndex();
            newsgroupString = subscribedNewsgroups[ position ].getNewsgroupName();
        }
        if( newsgroupString.length() == 0 )
        {
            //System.out.println( "Please enter a newsgroup name" );
            newsgroupComboBox.requestFocus();
            JOptionPane.showMessageDialog( window, "Please enter a newsgroup name", "Input Error", JOptionPane.ERROR_MESSAGE );
            return false;
        }


        // hostTextField must not be blank (format?)
        String  hostString = hostTextField.getText();
                if( hostString.length() == 0 )
                {
            //System.out.println( "Please enter a hostname" );
            hostTextField.requestFocus();
            JOptionPane.showMessageDialog( window, "Please enter a hostname", "Input Error", JOptionPane.ERROR_MESSAGE );
                        return false;
                }


        // replyTextField must have <string>@<string>.<string>
        // (string at least 2 chars long)
        // consider <s>.<s>@<s>.<s>.<s> format? (array of dot positons?)
        String replyString = replyTextField.getText();
        int atPos = replyString.indexOf( "@" );
        int dotPos = replyString.lastIndexOf( "." );
        int length = replyString.length();
        //System.out.println( "length: " + length + "\n atPos: " + atPos + "\n dotPos: " + dotPos );
        if( length == 0 || atPos == -1 || dotPos == -1 || atPos < 2 || dotPos < atPos || dotPos + 2 == length || atPos + 2 == dotPos || atPos != replyString.lastIndexOf( "@" ) || replyString.indexOf(" ") != -1 )
        {
            //System.out.println( "Please enter a valid reply to email address" );
            replyTextField.requestFocus();
            JOptionPane.showMessageDialog( window, "Please enter a valid reply to email address", "Input Error", JOptionPane.ERROR_MESSAGE );
            return false;
        }


        // subjectTextField must not be blank?
        String subjectString = subjectTextField.getText();
        if( subjectString.length() == 0 )
        {
            //System.out.println( "Please enter subject title" );
            subjectTextField.requestFocus();
            JOptionPane.showMessageDialog( window, "Please enter subject title", "Input Error", JOptionPane.ERROR_MESSAGE );
            return false;
        }

        // details are valid
        return true;
    }

}
