//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
import UIKit



// DocumentController is the main viewer in the app, it displayes the selected
// documents and holds a top entry to view the properties as well as a normal
// menu to handle global actions
// It is a delegate class to recieve Menu events as well as file handling events
class DocumentController: UIViewController, MenuDelegate, UIDocumentBrowserViewControllerDelegate
{
    // Handling of DocumentController
    // this is normal functions every controller must implement


    // called once controller is loaded
    override func viewDidLoad()
    {
        super.viewDidLoad()
        //FIXME
        // we should load the document types, so we can use them for the open call
        // let path = Bundle.main.path(forResource: "Info", ofType: "plist")
        // let dict = NSDictionary(contentsOfFile: path!)
        // let tableData = dict!.object(forKey: "CFBundleDocumentTypes")
    }



    // called when there is a memory constraint
    override func didReceiveMemoryWarning()
    {
        super.didReceiveMemoryWarning()
        // not used in this App
    }



    // Handling of Background (hipernate)
    // iOS is not true multitasking, only 1 app can be active (foreground) at any time,
    // therefore apps frequently are moved to the background.
    // background really means hipernate by terminating all threads and solely keep the
    // data

    // Moving to hipernate
    public func Hipernate() -> Void
    {
        BridgeLOkit_Hipernate()
    }



    // Moving back to foreground
    public func LeaveHipernate() -> Void
    {
        BridgeLOkit_LeaveHipernate()
    }



    // handling of PropertiesController
    // The PropertiesController is a left sidebar, that will scroll in when activated
    // The Controller handles manipulation of properties in the document

    // Activate/Deactivate PropertiesController (from navigationController, see storyboard)
    @IBAction func doProperties(_ sender: UIBarButtonItem)
    {
        // Check if deactivation
        if (sender.tag == 99) {
            // Deactivate

            // Mark it as deactivated (it stays loaded)
            sender.tag = 0;

            // get handle of PropertiesController
            let viewMenuBack : UIView = view.subviews.last!

            // Blend out sidebar
            UIView.animate(withDuration: 0.3, animations: { () -> Void in
                var frameMenu : CGRect = viewMenuBack.frame
                frameMenu.origin.x = -1 * UIScreen.main.bounds.size.width
                viewMenuBack.frame = frameMenu
                viewMenuBack.layoutIfNeeded()
                viewMenuBack.backgroundColor = UIColor.clear
                }, completion: { (finished) -> Void in
                    viewMenuBack.removeFromSuperview()
                })
        }
        else {
            // Activate

            // Mark as activated
            sender.isEnabled = false
            sender.tag = 99

            // make instance of PropertiesController
            let prop : PropertiesController = self.storyboard!.instantiateViewController(
                withIdentifier: "PropertiesController") as! PropertiesController
            view.addSubview(prop.view)
            addChildViewController(prop)
            prop.view.layoutIfNeeded()
            prop.view.frame=CGRect(x: 0 - UIScreen.main.bounds.size.width,
                                   y: 0,
                                   width: UIScreen.main.bounds.size.width,
                                   height: UIScreen.main.bounds.size.height);

            // Blend in sidebar
            UIView.animate(withDuration: 0.3, animations: { () -> Void in
                prop.view.frame=CGRect(x: 0,
                                       y: 0,
                                       width: UIScreen.main.bounds.size.width,
                                       height: UIScreen.main.bounds.size.height);
                sender.isEnabled = true
                }, completion:nil)
        }
    }





    // var currentDocumentName : String?


    internal func documentBrowser(_ controller: UIDocumentBrowserViewController,
                                  didRequestDocumentCreationWithHandler importHandler: @escaping (URL?, UIDocumentBrowserViewController.ImportMode) -> Void)
    {
        // Asks the delegate to create a new document.
    }

    internal func documentBrowser(_ controller: UIDocumentBrowserViewController,
                                  didImportDocumentAt sourceURL: URL,
                                  toDestinationURL destinationURL: URL)
    {
        // Tells the delegate that a document has been successfully imported.
        //FIX BridgeLOkit_open("jan");
        //FIX BridgeLOkit_ClientCommand("jan");

    }

    internal func documentBrowser(_ controller: UIDocumentBrowserViewController,
                                  failedToImportDocumentAt documentURL: URL,
                                  error: Error?)
    {
        // Tells the delegate that the document browser failed to import the specified document.
    }

    internal func documentBrowser(_ controller: UIDocumentBrowserViewController,
                                  didPickDocumentURLs documentURLs: [URL])
    {
        // Tells the delegate that the user has selected one or more documents.
        //FIX BridgeLOkit_open("jan");
        //FIX BridgeLOkit_ClientCommand("jan");
    }

    @IBOutlet weak var janTest: UILabel!

    // Last stop before displaying popover
    override func prepare(for segue: UIStoryboardSegue, sender: Any?)
    {
        if segue.identifier == "showActions" {
            let vc = segue.destination as! DocumentActions
            vc.delegate = self

            // JIX, TO BE CHANGED
            vc.isDocActive = true
        }
    }



    func actionMenuSelected(_ tag : Int)
    {
        switch tag
        {
            case 1: // Open...
                // let openMenu = UIDocumentPickerViewController(documentTypes: ["public.data"], in: .open)
                let openMenu = UIDocumentBrowserViewController()
                //penMenu.allowsDocumentCreation = true
                // UIDocumentBrowserViewController.ImportMode = UIDocumentBrowserViewController.ImportMode.none // copy, move
                //openMenu.InterfaceStyle = UIDocumentPickerViewController.dark
                openMenu.delegate = self
                self.present(openMenu, animated: true, completion: nil)
                print("menu Open... to be done")

            case 2: // Properties
                print("menu Properties to be done")

            case 3: // Save
                print("menu Save to be done")

            case 4: // Save as...
                print("menu Save as... to be done")

            case 5: // Save as PDF...
                print("menu Save as PDF... to be done")

            case 6: // Print...
                print("menu Print... to be done")

            default: // should not happen
                print("unknown menu" + String(tag))
        }
    }
}



// Protocol for action popover callback
protocol MenuDelegate
{
    func actionMenuSelected(_ tag : Int)
}



class DocumentActions: UITableViewController
{
    // Pointer to callback class
    var delegate  : MenuDelegate?
    var isDocActive : Bool = false

    // Calling class might enable/disable each button
    @IBAction func actionMenuSelect(_ sender: UIButton)
    {
        dismiss(animated: false)
        delegate?.actionMenuSelected(sender.tag)
    }

    @IBOutlet weak var buttonProperties: UIButton!
    @IBOutlet weak var buttonNew: UIButton!
    @IBOutlet weak var buttonOpen: UIButton!
    @IBOutlet weak var buttonSave: UIButton!
    @IBOutlet weak var buttonSaveAs: UIButton!
    @IBOutlet weak var buttonSaveAsPDF: UIButton!
    @IBOutlet weak var buttonPrint: UIButton!
    @IBOutlet weak var buttonCopy: UIButton!
    @IBOutlet weak var buttonMove: UIButton!
    @IBOutlet weak var buttonDelete: UIButton!

    override func viewDidLoad()
    {
        super.viewDidLoad()
        buttonSave.isEnabled = isDocActive
        buttonSaveAs.isEnabled = isDocActive
        buttonSaveAsPDF.isEnabled = isDocActive
        buttonPrint.isEnabled = isDocActive
    }
}


