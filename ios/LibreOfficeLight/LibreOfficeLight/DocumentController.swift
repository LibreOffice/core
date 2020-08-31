//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
import UIKit


// DocumentController is the main viewer in the app, it displays the selected
// documents and holds a top entry to view the properties as well as a normal
// menu to handle global actions
// It is a delegate class to receive Menu events as well as file handling events
class DocumentController: UIViewController, MenuDelegate, UIDocumentBrowserViewControllerDelegate
{
    var document: DocumentHolder? = nil

    var documentView: DocumentTiledView? = nil
    var documentOverlaysView: DocumentOverlaysView? = nil

    // *** Handling of DocumentController
    // this is normal functions every controller must implement


    // holds known document types
    var KnownDocumentTypes : [String] = []

    var zeroInsets: UIEdgeInsets = .zero

    @IBOutlet weak var scrollView: UIScrollView!
    @IBOutlet weak var mask: UIView!
    @IBOutlet weak var progressBar: UIProgressView!
    @IBOutlet weak var searchBar: UISearchBar!
    @IBOutlet weak var buttonScrollView: ButtonScrollView!

    deinit
    {
        NotificationCenter.default.removeObserver(self)
    }

    // called once controller is loaded
    override func viewDidLoad()
    {
        super.viewDidLoad()

        // loading known document types, so we can use them for the open call
        let path = Bundle.main.path(forResource: "Info", ofType: "plist")
        let plist = NSDictionary(contentsOfFile: path!)
        for dict in (plist!.object(forKey: "UTExportedTypeDeclarations") as! [NSDictionary]) +
                    (plist!.object(forKey: "UTImportedTypeDeclarations") as! [NSDictionary]) {
            let x = ((dict["UTTypeTagSpecification"]  as! NSDictionary)["public.filename-extension"] as! NSArray)
            KnownDocumentTypes.append( x[0] as! String )
        }
        LOKitThread.instance.progressDelegate = self
    }

    override func viewWillAppear(_ animated: Bool)
    {
        super.viewWillAppear(animated)
        registerKeyboardNotifications()
    }

    override func viewDidAppear(_ animated: Bool)
    {
        super.viewDidAppear(animated)

        // Always load the 'welcome' file, as per the android app
        let res = Bundle.main.url(forResource: "welcome", withExtension: "odt")

        // uncomment for test data in resources until the doc picker works properly
        //let res = Bundle.main.url(forResource: "testdata/2", withExtension: "xlsx")

        if let exampleDoc = res
        {
            self.doOpen(exampleDoc)
        }
    }

    // called when there is a memory constraint
    override func didReceiveMemoryWarning()
    {
        super.didReceiveMemoryWarning()
        // not used in this App
    }

    @IBAction func searchIconTapped(_ sender: Any)
    {
        searchBar.isHidden = !searchBar.isHidden
        if (!searchBar.isHidden)
        {
            searchBar.becomeFirstResponder()
        }
    }


    // *** Handling of Background (hipernate)
    // iOS is not true multitasking, only 1 app can be active (foreground) at any time,
    // therefore apps frequently are moved to the background.
    // background really means hipernate by terminating all threads and solely keep the
    // data



    // Moving to hipernate
    public func Hipernate() -> Void
    {
        //BridgeLOkit_Hipernate() // FIXME
    }



    // Moving back to foreground
    public func LeaveHipernate() -> Void
    {
        //BridgeLOkit_LeaveHipernate() // FIXME
    }



    // *** handling of PropertiesController
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



    // *** Handling of menu popover
    // the menu contains all global functions and use seque/delegate



    var currentDocumentName : String? = nil



    // Last stop before displaying popover
    override func prepare(for segue: UIStoryboardSegue, sender: Any?)
    {
        // "showActions" is the name of the popover menu, see storyboard
        if segue.identifier == "showActions" {
            let vc = segue.destination as! DocumentActions
            vc.delegate = self
            vc.isDocActive = (currentDocumentName != nil)
        }
    }



    // Delegate call from menu (see protocol MenuDelegate)
    func actionMenuSelected(_ tag : Int)
    {
        // a tag can sadly enough only be a number and not a string,
        // whenever adding a menu entry, it (of course) needs to be added
        // to the Document actions scene in storyboard and assigned a tag number
        // the tag number must be repeated in the following switch
        // BE CAREFUL to keep the tags synchronized (manually)
        switch tag
        {
        case 1: // Open...
                startOpenDocument()

        case 2: // Properties
                showProperties()

        case 3: // Save
                doSave()

        case 4: // Close...
                doClose()

        case 5: // Save as...
                doSaveAs()

        case 6: // Save as PDF...
                doSaveAsPDF()

        case 7: // Print...
                startPrint()

        default: // should not happen
                 print("unknown menu" + String(tag))
        }
    }



    // *** handling of menu actions
    // This is the real base of the application

    var openMenu : UIDocumentBrowserViewController? = nil

    // Load document into LibreOfficeKit and present it
    internal func startOpenDocument()
    {
        openMenu = UIDocumentBrowserViewController()
        openMenu?.allowsDocumentCreation = true
        openMenu?.browserUserInterfaceStyle = UIDocumentBrowserViewController.BrowserUserInterfaceStyle.dark
        openMenu?.delegate = self
        self.present(openMenu!, animated: true, completion: nil)
    }



    // Show document properties (new overloaded page)
    internal func showProperties()
    {
        //FIXME
        print("menu Properties to be done")
    }



    // Save current document
    internal func doSave()
    {
        //FIXME
        print("menu Save to be done")
    }



    // Close current document (without saving)
    internal func doClose()
    {
        //FIXME
        print("menu Close to be done")
    }



    // make a copy of current document, and save
    internal func doSaveAs()
    {
        //FIXME
        print("menu Save as... to be done")
    }



    // save current document as PDF
    internal func doSaveAsPDF()
    {
        //FIXME
        print("menu Save as PDF... to be done")
    }



    // print current document
    internal func startPrint()
    {
        //FIXME
        print("menu Print... to be done")
    }



    // *** Handling of DocumentViewController delegate functions
    // this handles open/create/copy/delete document



    // Create an empty document, and present it
    internal func documentBrowser(_ controller: UIDocumentBrowserViewController,
                                  didRequestDocumentCreationWithHandler importHandler: @escaping (URL?,
                                  UIDocumentBrowserViewController.ImportMode) -> Void)
    {
        //FIXME
    }



    // import (copy from iCloud to iPad) document, open it and present it
    internal func documentBrowser(_ controller: UIDocumentBrowserViewController,
                                  didImportDocumentAt sourceURL: URL,
                                  toDestinationURL destinationURL: URL)
    {
        //FIXME
    }



    // Import failed, inform user
    internal func documentBrowser(_ controller: UIDocumentBrowserViewController,
                                  failedToImportDocumentAt documentURL: URL,
                                  error: Error?)
    {
        //FIXME
    }



    // open document and present it
    internal func documentBrowser(_ controller: UIDocumentBrowserViewController,
                                  didPickDocumentURLs documentURLs: [URL])
    {
        openMenu?.dismiss(animated: true, completion: nil)
        openMenu = nil
        doOpen(documentURLs[0])
    }



    // *** Handling of document (open/print)



    // Real open and presentation of document
    public func doOpen(_ docURL : URL)
    {
        LOKitThread.instance.documentLoad(url: docURL.absoluteString)
        {
            doc, error in

            if let document = doc
            {

                runOnMain
                {
                    self.setDocument(doc: document)
                }
            }
            else
            {
                // TODO - alert user of failure

            }
        }

        /* FIXME
        BridgeLOkit_Sizing(4, 4, 256, 256);
 */
    }

    /// Sets the document to use and set's up it's view. Should be called on the main thread
    public func setDocument(doc: DocumentHolder)
    {
        if let _ = self.document
        {
            // TODO - cleanup
            self.document = nil
        }
        if let existingView = self.documentView
        {
            existingView.removeFromSuperview()
            self.documentView = nil // forces the close of the view and it's held documents before we setup the new one
        }
        // also remove current overlays and start fresh
        documentOverlaysView?.removeFromSuperview()

        // setup the new doc view
        self.document = doc
        // setup delegates
        doc.searchDelegate = self

        let frameToUse = self.scrollView.frame

        let docView = DocumentTiledView(frame: frameToUse, document: doc, scale: 1.0)

        self.scrollView.addSubview(docView)
        self.scrollView.contentSize = docView.frame.size
        self.documentView = docView

        // overlay view
        let overlay = DocumentOverlaysView(docTiledView: docView)
        docView.addSubview(overlay)
        self.documentOverlaysView = overlay

        // button view - used for spreadsheet tabs
        if doc.isSpreadsheet
        {
            buttonScrollView.isHidden = false
            buttonScrollView.setButtonLabels(labels: doc.partNames)
            buttonScrollView.buttonClickedCallback = {
                [weak self] index in
                self?.document?.async {
                    $0.setPart(nPart: Int32(index))
                    runOnMain {
                        self?.documentView?.setNeedsDisplay()
                    }
                }
            }
            // make room for the scroll view
            zeroInsets = UIEdgeInsets(top: 0, left: 0, bottom: buttonScrollView.height, right: 0)
        }
        else
        {
            zeroInsets = .zero
            buttonScrollView.isHidden = true
        }
        scrollView.contentInset = zeroInsets

        // debugging view borders
        /*
        self.scrollView.layer.borderColor = UIColor.red.cgColor
        self.scrollView.layer.borderWidth = 1.0
        docView.layer.borderColor = UIColor.green.cgColor
        docView.layer.borderWidth = 1.0
        */
    }

    // MARK: - UIScrollViewDelegate
}

extension DocumentController: UIScrollViewDelegate
{
    // return a view that will be scaled. if delegate returns nil, nothing happens
    func viewForZooming(in scrollView: UIScrollView) -> UIView?
    {
        return self.documentView
    }

    // called before the scroll view begins zooming its content
    func scrollViewWillBeginZooming(_ scrollView: UIScrollView, with view: UIView?)
    {
        print("scrollViewWillBeginZooming currentScale=\(scrollView.zoomScale)")
    }

    // scale between minimum and maximum. called after any 'bounce' animations
    func scrollViewDidEndZooming(_ scrollView: UIScrollView, with view: UIView?, atScale scale: CGFloat)
    {
        print("scrollViewDidEndZooming scale=\(scale)")
        self.documentView?.scrollViewDidEndZooming(scrollView, with: view, atScale: scale)
    }
}

    // MARK: -  UIKeyInput
//    public var hasText: Bool
//    {
//        true
//    }
//
//
//    public func insertText(_ text: String)
//    {
//
//    }
//
//    public func deleteBackward()
//    {
//
//    }

extension DocumentController: ProgressDelegate
{
    // MARK: - ProgressDelegate
    func statusIndicatorStart()
    {
        self.mask?.isHidden = false
        self.progressBar?.isHidden = false
        self.progressBar?.progress = 0.0
    }

    func statusIndicatorFinish()
    {
        // what would be nice would be to be able to wait until the initial tiles have rendered...
        self.mask?.isHidden = true
        self.progressBar?.isHidden = true
    }

    func statusIndicatorSetValue(value: Double)
    {
        self.progressBar?.progress = Float(value) / 100.0
    }
}

extension DocumentController: UISearchBarDelegate
{
    // called when text changes (including clear)
    func searchBar(_ searchBar: UISearchBar, textDidChange searchText: String)
    {

    }


    // called when keyboard search button pressed
    func searchBarSearchButtonClicked(_ searchBar: UISearchBar)
    {
        if let text = searchBar.text
        {
            if text.count > 0
            {
                document?.search(searchString: text, forwardDirection: true, from: CGPoint(x:0, y:0) )
            }
        }
    }

    func searchBarCancelButtonClicked(_ searchBar: UISearchBar)
    {
        searchBar.isHidden = true
    }
}

extension DocumentController: SearchDelegate
{
    func searchNotFound()
    {
        // TODO: tell user somehow
        self.documentOverlaysView?.clearSearchResults()
    }

    func searchResultSelection(searchResults: SearchResults)
    {
        self.documentOverlaysView?.setSearchResults(searchResults: searchResults)
    }
}

/// Keyboard notifications
extension DocumentController
{

    func registerKeyboardNotifications()
    {
        NotificationCenter.default.addObserver(self,
                                               selector: #selector(keyboardWillShow(notification:)),
                                               name: NSNotification.Name.UIKeyboardWillShow,
                                               object: nil)
        NotificationCenter.default.addObserver(self,
                                               selector: #selector(keyboardWillHide(notification:)),
                                               name: NSNotification.Name.UIKeyboardWillHide,
                                               object: nil)
    }

    @objc func keyboardWillShow(notification: NSNotification)
    {

        let userInfo: NSDictionary = notification.userInfo! as NSDictionary
        guard let keyboardInfo = userInfo[UIKeyboardFrameEndUserInfoKey] as? NSValue else { return }
        print(userInfo)
        let keyboardSize = keyboardInfo.cgRectValue.size
        print("keyboardWillShow \(keyboardSize)")
        let contentInsets = UIEdgeInsets(top: 0, left: 0, bottom: keyboardSize.height, right: 0)
        scrollView.contentInset = contentInsets
        scrollView.scrollIndicatorInsets = contentInsets
    }

    @objc func keyboardWillHide(notification: NSNotification)
    {
        print("keyboardWillHide")
        scrollView.contentInset = zeroInsets
        scrollView.scrollIndicatorInsets = zeroInsets
    }
}
