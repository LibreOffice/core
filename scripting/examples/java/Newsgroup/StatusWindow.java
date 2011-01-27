import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.border.*;

public class StatusWindow extends JFrame
{

    private JProgressBar progressBar = null;
    private JTextField statusLabel = null;
    private JButton cancelButton = null;
    private JFrame statusWindow = null;
    private PostNewsgroup mainWindow = null;

    private final int MAXPROGRESS = 13;
    private final int MINPROGRESS = 0;


    public StatusWindow( PostNewsgroup mw, String title, int parentX, int parentY )
    {
        this.setTitle( title );
        this.setLocation( parentX + 100, parentY + 100 );
        statusWindow = this;
        mainWindow = mw;

        mainWindow.enableButtons( false );

        statusWindow.addWindowListener( new WindowAdapter()
        {
            public void windowClosing( WindowEvent event ) {
                mainWindow.enableButtons( true );
            }
        });

        progressBar = new JProgressBar();
        progressBar.setStringPainted( true );
        progressBar.setMaximum( MAXPROGRESS );
        progressBar.setMinimum( MINPROGRESS );
        progressBar.setSize( 30, 400 );

        JLabel progLabel = new JLabel( "Progress:" );

        JPanel progressPanel = new JPanel();
        progressPanel.setLayout( new BorderLayout( 10, 0 ) );
        progressPanel.add( progLabel, "West" );
        progressPanel.add( progressBar, "East" );

        statusLabel = new JTextField();
        statusLabel.setColumns( 25 );
        statusLabel.setEditable( false );
        statusLabel.setBorder( null );
        //statusLabel.setBorder( LineBorder.createGrayLineBorder() );
        JPanel statusPanel = new JPanel();
        //statusPanel.setBorder( LineBorder.createBlackLineBorder() );
        statusPanel.setLayout( new BorderLayout() );
        statusPanel.add( statusLabel, "West" );

        cancelButton = new JButton( "Cancel" );
        cancelButton.setSize( 30, 100 );
        cancelButton.setEnabled( false );
        cancelButton.addActionListener( new ActionListener()
        {
                        public void actionPerformed( ActionEvent event ) {
                                // cancelling actions
                mainWindow.enableButtons( true );
                statusWindow.dispose();
                        }
                });

        JPanel buttonPanel = new JPanel();
        buttonPanel.setLayout( new BorderLayout( 0, 5 ) );
        buttonPanel.add( cancelButton, "East" );
        buttonPanel.add( new JSeparator( SwingConstants.HORIZONTAL ), "North" );

        Container container = getContentPane();
        container.setLayout( new GridBagLayout() );
        GridBagConstraints constraints = new GridBagConstraints();
        constraints.fill = GridBagConstraints.BOTH;

        constraints.gridx = 0;
        constraints.gridy = 0;
        constraints.gridwidth = 1;
        constraints.gridheight = 1;
        constraints.insets = new Insets( 15, 15, 10, 15 );
        container.add( progressPanel, constraints );

        constraints.gridx = 0;
        constraints.gridy = 1;
        constraints.gridwidth = 1;
        constraints.gridheight = 1;
        constraints.insets = new Insets( 10, 15, 10, 15 );
        container.add( statusPanel, constraints );

        constraints.gridx = 0;
        constraints.gridy = 2;
        constraints.gridwidth = 1;
        constraints.gridheight = 1;
        constraints.insets = new Insets( 10, 15, 5, 15 );
        container.add( buttonPanel, constraints );

        this.pack();
        this.setResizable( false );
        //this.setVisible( true );

    }


    public void setStatus( int progress, String status )
    {
        progressBar.setValue( progress );
        statusLabel.setText( status );
        statusLabel.setToolTipText( status );
        if( progress == MAXPROGRESS )
        {
                    cancelButton.setEnabled( true );
                    cancelButton.setText( "Close" );
        }
        update( getGraphics() );
        mainWindow.update( mainWindow.getGraphics() );
    }


    public void enableCancelButton( boolean enable )
    {
        if( enable )
        {
            cancelButton.setEnabled( true );
            cancelButton.setText( "Finish" );
        }
        else
        {
            cancelButton.setEnabled( false );
            cancelButton.setText( "Cancel" );
        }

    }

}
