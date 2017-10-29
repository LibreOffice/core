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
    // Show sidemenu (part of documentcontroller)
    @IBAction func doProperties(_ sender: UIBarButtonItem)
    {
        if (sender.tag == 99) {
            sender.tag = 0;

            let viewMenuBack : UIView = view.subviews.last!

            UIView.animate(withDuration: 0.3, animations: { () -> Void in
                var frameMenu : CGRect = viewMenuBack.frame
                frameMenu.origin.x = -1 * UIScreen.main.bounds.size.width
                viewMenuBack.frame = frameMenu
                viewMenuBack.layoutIfNeeded()
                viewMenuBack.backgroundColor = UIColor.clear
                }, completion: { (finished) -> Void in
                    viewMenuBack.removeFromSuperview()
                })
            return
        }

        sender.isEnabled = false
        sender.tag = 99

        let properties : PropertiesController = self.storyboard!.instantiateViewController(withIdentifier: "PropertiesController") as! PropertiesController
        view.addSubview(properties.view)
        addChildViewController(properties)
        properties.view.layoutIfNeeded()

        properties.view.frame=CGRect(x: 0 - UIScreen.main.bounds.size.width, y: 0, width: UIScreen.main.bounds.size.width, height: UIScreen.main.bounds.size.height);

        UIView.animate(withDuration: 0.3, animations: { () -> Void in
            properties.view.frame=CGRect(x: 0, y: 0, width: UIScreen.main.bounds.size.width, height: UIScreen.main.bounds.size.height);
            sender.isEnabled = true
            }, completion:nil)
    }



    public func Hipernate() -> Void
    {
    }

    public func LeaveHipernate() -> Void
    {
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


    override func viewDidLoad()
    {
        super.viewDidLoad()
        // Do any additional setup after loading the view.

        // let path = Bundle.main.path(forResource: "Info", ofType: "plist")
        // let dict = NSDictionary(contentsOfFile: path!)
        // let tableData = dict!.object(forKey: "CFBundleDocumentTypes")
    }



    override func didReceiveMemoryWarning()
    {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
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


