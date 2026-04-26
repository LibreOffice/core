# Contributing to Collabora Online

:+1::tada: First off, thanks for taking the time to contribute! :tada::+1:

The following is a set of rules and guidelines for contributing to Collabora Online. Please feel free to propose changes to this document in a pull request.

## Submitting issues

If you have questions about how to install or use Collabora Online, please direct these to our [forum][forum].
If you have issues or questions about Collabora Online development, you may join us on [IRC][irc] or [Telegram][telegram].

### Guidelines

- Please search the existing issues first, it's likely that your issue was already reported or even fixed.
  - Go to the main page of the repository, click "issues" and type any word in the top search/command bar.
  - You can also filter by appending e. g. "state:open" to the search string.
  - More info on [search syntax within github](https://help.github.com/articles/searching-issues)
- **SECURITY**: Report any potential security bug to us following our [security policy](https://github.com/CollaboraOnline/online/security/policy) instead of filing an issue in our bug tracker.

- [Report the issue][report] using one of our templates, they include all the information we need to track down the issue.

Help us to maximize the effort we can spend fixing issues and adding new features, by not reporting duplicate issues.

[report]: https://github.com/CollaboraOnline/online/issues/new/choose
[forum]: https://forum.collaboraonline.com/
[irc]: https://web.libera.chat/?channels=cool-dev
[telegram]: https://t.me/CollaboraOnline

## Contributing to Source Code

Thanks for wanting to contribute source code to Collabora Online. You rock!

Development and code review happen on our Gerrit instance at **https://gerrit.collaboraoffice.com**. Pull requests on GitHub will be automatically closed — please use Gerrit instead.

1. **Sign in to Gerrit**: Go to https://gerrit.collaboraoffice.com and sign in with your GitHub account
2. **Add your SSH key**: Go to https://gerrit.collaboraoffice.com/settings/#SSHKeys
3. **Clone the repo**:
   ```bash
   git clone ssh://YOUR_USERNAME@gerrit.collaboraoffice.com:29418/online
   cd online
   ```
4. **Install the commit-msg hook** (adds Change-Id automatically):
   ```bash
   scp -p -P 29418 YOUR_USERNAME@gerrit.collaboraoffice.com:hooks/commit-msg .git/hooks/
   ```
5. Build it ([on Linux](https://collaboraonline.github.io/post/build-code/) or [on any platform](https://forum.collaboraonline.com/t/start-developing-cool-on-any-platform-in-5-minutes/52))
6. Grab one of our newcomer-friendly [easy hacks](https://collaboraonline.github.io/post/easyhacks/)
7. **Submit your patch for review**:
   ```bash
   git push origin HEAD:refs/for/main
   ```

And if you get stuck at any point, just drop by one of our [communication channels](https://collaboraonline.github.io/post/communicate/).

### Sign your work

We use the Developer Certificate of Origin (DCO) as a additional safeguard for the Collabora Online project. This is a well established and widely used mechanism to assure contributors have confirmed their right to license their contribution under the project's license. Please read [README.CONTRIBUTING.md](README.CONTRIBUTING.md). If you can certify it, then just add a line to every git commit message:

```
  Signed-off-by: Random J Developer <random@developer.example.org>
```

Use your real name (sorry, no pseudonyms or anonymous contributions). If you set your `user.name` and `user.email` git configs, you can sign your commit automatically with `git commit -s`.

### Change-Id

We use [Change-Ids](https://gerrit-review.googlesource.com/Documentation/user-changeid.html) in Collabora Online. If you installed the commit-msg hook from Gerrit (step 4 above), the Change-Id is added automatically to every commit.

### For non-technical people

Hello, and thanks for stopping by! If you want to work on something like an icon or a label fix but aren't comfortable with the command line, you can still contribute via GitHub:

1. [Fork the repo](https://github.com/CollaboraOnline/online/fork) and make your changes
2. Instead of opening a pull request, reach out to us on one of our [communication channels](https://collaboraonline.github.io/post/communicate/) and a developer will help you submit the patch via Gerrit

### AI Policy

We consider AI tools that help programming just one more tool that, if used judiciously, can help
developers learn to code. Volunteers using such AI tools are permitted to get good ideas, but you
are responsible for the resulting code: you should be able to explain each hunk of the diff you have
written based on the generated samples. Submitting patches without understanding their content,
copying AI generated code verbatim, or not testing the result before submitting would create risk,
and waste reviewer time and is forbidden.

## Translations

Please submit translations via [Weblate](https://hosted.weblate.org/projects/collabora-online).
